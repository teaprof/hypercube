from operator import mul
from hypercube import *
import itertools, math
import tqdm
import multiprocessing, ctypes, contextlib
import time, csv, json, enum

results_filename= "results.csv"

def createFilesForSubmit(maxMemoryMB, maxPoints = int(1e+9)):
    maxcells = maxMemoryMB*1024*1024//HypercubeProblem.bytes_per_cell
    rng_type = [1]
    bits_repack_sample_size = [8, 16, None]
    srcLittleEndian = [False, True]
    dstLittleEndian = [False, True]
    dimensions = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]    
    m_intervals_per_dim = [10, 20, 30, 40, 1e+2, 2e+2, 5e+2, 1e+3, 1e+4, 1e+5, 1e+6, 1e+7]
    m_intervals_per_dim.extend([2**n for n in range(1, 40)])
    n_points_per_cell = [10, 20, 50, 1e+2, 1e+3, 1e+4, 1e+5, 1e+6]
    strides = [None] # Node for stride == dim
    strides.extend(range(1, 10)) 

    # convert to int
    m_intervals_per_dim = [int(n) for n in m_intervals_per_dim]
    n_points_per_cell = [int(n) for n in n_points_per_cell]
    jobs = []
    for rng, sample_size, srcLE, dstLE, dim, m, n_per_cell, stride in itertools.product(rng_type, bits_repack_sample_size, srcLittleEndian, dstLittleEndian, dimensions, m_intervals_per_dim, n_points_per_cell, strides):
        if sample_size:
            bits_repack = BitsRepack(sample_size, srcLE, dstLE)
        else:
            if srcLE == True and dstLE == True:
                #only one combination for bits_repack = None
                bits_repack = None
            else:
                continue
        if stride == None and (dim in strides):
            # this combination is the same as stride == dim
            continue
        rng = RNG(rng, 0)
        total_cells = m ** dim
        if sample_size and 2**sample_size / m < 100:            
            continue
        total_points = n_per_cell*total_cells
        problem = HypercubeProblem(dim, m, total_points, stride)
        if total_cells <= maxcells and total_points <= maxPoints:
            nsubtasks = math.ceil(total_cells / maxcells)
            for cursubtask in range(nsubtasks):
                subtask = Subtask(cursubtask, nsubtasks)
                job = HypercubeJob(rng, bits_repack, problem, subtask)
                jobs.append(job)
        else:
            #print(total_cells*4)
            pass

    #remove duplicates and check for cash collisions
    if len(jobs) > 0:
        all_hashes = [hash(j) for j in jobs]
        all_hashes_with_idx = sorted(zip(all_hashes, range(len(all_hashes))), key = lambda x: x[0])
        idx_to_remove = []
        prev_hash = all_hashes_with_idx[0][0]
        prev_idx =  all_hashes_with_idx[0][1]
        for n in range(1, len(all_hashes_with_idx)): 
            cur_hash = all_hashes_with_idx[n][0]
            cur_idx =  all_hashes_with_idx[n][1]
            if prev_hash == cur_hash:
                assert jobs[cur_idx] == jobs[prev_idx], "Hash collision detected: two different jobs have the same hash" 
                idx_to_remove.append(cur_idx)
            prev_hash, prev_idx = cur_hash, cur_idx
        idx_to_remove = sorted(idx_to_remove, reverse=True)
        for idx in idx_to_remove:
            jobs.pop(idx)
        #Final check for collisions
        all_hashes = [hash(j) for j in jobs]
        assert len(all_hashes) == len(set(all_hashes)) #should always be passed

    return jobs



class Resource:
    def __init__(self, value):
        self.resource = multiprocessing.Value(ctypes.c_uint32)
        self.resource.value = value
        self.condition = multiprocessing.Condition()
        
    def _acquire(self, amount):
        with self.condition:
            self.condition.wait_for(lambda: self.resource.value >= amount)            
            self.resource.value -= amount
            print(f"Borrowing {amount}, left {self.resource.value}")
            
    def _release(self, amount):
        with self.condition:
            self.resource.value += amount
            self.condition.notify()
            
    @contextlib.contextmanager
    def borrow(self, amount):
        self._acquire(amount)
        try:            
            yield
        finally:
            self._release(amount)
            
class JobsTracker:
    class JobStatus(enum.Enum):
        Pending = 0
        Running = 1
        Ready = 2
        
    def __init__(self):
        self.mutex = multiprocessing.Lock()
        self.map = multiprocessing.Manager().dict()

    def add_if_not_exists(self, key, value: JobStatus = JobStatus.Pending):
        with self.mutex:
            if key not in self.map:
                self.map[key] = value
                return True
            else:
                return False
    
    def compare_and_swap(self, key, expected: JobStatus, newvalue: JobStatus):
        with self.mutex:
            assert key in self.map
            if self.map[key] == expected:
                self.map[key] = newvalue
                return expected
            else:
                return self.map[key]

    def markAsFinishedByHash(self, hashes: List[int]):
        """
        The first way to skip the jobs that are already solved.
        Also, you can save the state and later reload it (see save and load functions)
        """
        with self.mutex:
            for h in hashes:
                self.map[h] = JobsTracker.JobStatus.Ready

    def getUnfinishedJobCount(self, jobs):
        counter = 0
        with self.mutex:  
            for j in jobs:
                key = hash(j)
                if key in self.map.keys():
                    if self.map[key] == JobsTracker.JobStatus.Ready:
                        continue
                counter += 1
        return counter
                
    @staticmethod
    def load(filename):
        try:
            with open(filename, "rb") as f:
                obj = pickle.load(f)
        except (FileNotFoundError, EOFError) as err:
            return JobsTracker() #empty object
        for key in obj.map.keys():
            if obj.map[key] == JobsTracker.JobStatus.Running:
                obj.map[key] = JobsTracker.JobStatus.Pending
        return obj
            
    def save(self, filename):
        with open(filename, "wb") as f:
            pickle.dump(jobsTracker, f)
            
    def __getstate__(self):
        with self.mutex:
            # create a local copy of dict from proxy object
            map = {}
            for key, value in self.map.items():
                map[key] = value
            res = {'map': map}
            return res

    def __setstate__(self, dict):
        self.mutex = multiprocessing.Lock()        
        self.map = multiprocessing.Manager().dict()
        for key, value in dict['map'].items():
            self.map[key] = value
        
            
class JobResults:
    def __init__(self):
        self.mutex = multiprocessing.Lock()
    
    def addResults(self, job, res, time_elapsed):
        with self.mutex:            
            # create header if file not exists
            try:                
                with open(results_filename, "x") as f:
                    #f.write(f"{'hash':>16} {'date':>10} {'time':>8} {'elapsed':>10} {'rng':>4} {'ofs':>4} {'bps':>4} {'srLE':>4} {'dsLE':>4} {'dim':>4} {'m':>10} {'stride':>6} {'N':>10} {'Mtot':>10} {'curt':>4} {'Ntsk':>4} {'sum':>20} {'sum2':>20} {'chi2cdf':>21} {'p_ok':>4}\n")
                    f.write("hash date time elapsed rng offset bitsPerSample srcLittleEndian dstLittleEndian dim m stride N Mtotal curtask Ntasks sum sum2 chi2cdf parameters_ok\n");
            except FileExistsError:
                pass
            # write new data to the file
            with open(results_filename, "a") as f:
                f.write(f"{hash(job):016x} {time.strftime('%Y.%m.%d %H:%M:%S')} {time_elapsed:10.2e} {repr(job)} {res.sum:20} {res.sum2:20} {res.chi2cdf():21.15e} {res.parameters_ok:4}\n")
                
                
    def loadFinishedJobsHashes(self):
        finished_jobs_hashes = []
        with self.mutex:
            try:
                with open(results_filename, "r") as f:
                    reader = csv.reader(f, delimiter=" ", )
                    next(reader, None) # skip header
                    for row in reader:
                        h = int(row[0], 16)
                        finished_jobs_hashes.append(h)
            except FileNotFoundError:
                pass
        return finished_jobs_hashes                                

memoryResource = Resource(0)
jobsTracker = JobsTracker()
jobsResults = JobResults()

def run(job: HypercubeJob, write_results = True): 
    # check if the job is not done yet
    h = hash(job)
    jobsTracker.add_if_not_exists(h, JobsTracker.JobStatus.Pending)
    #print(jobsTracker.map[h])
    if jobsTracker.compare_and_swap(h, JobsTracker.JobStatus.Pending, JobsTracker.JobStatus.Running) != JobsTracker.JobStatus.Pending:
        #print(f"Skipping job {h:016x}")
        return
    
    # lock memory for the job
    global memoryResource
    memMB = math.ceil(job.maxMemory() / 1024**2) # MB
    with memoryResource.borrow(memMB):
        #delay = min(10, job.problem.Npoints*1e-10 + 0.1)
        #print(f"job {h}: {job.problem.Npoints} -> {delay} secs, mem slots = {mem}")
        print(f"job {h}: {float(job.problem.Npoints):g} points, mem slots = {memMB}")
        #time.sleep(delay)
        print(job)
        t1 = time.time()
        res = job.run()
        t2 = time.time()        
        print(f"{h} finished: {job} in {t2-t1} seconds at {time.ctime()}")
    
    if write_results:
        jobsResults.addResults(job, res, t2 - t1)
    
    # mark the job as finished
    assert jobsTracker.compare_and_swap(h, JobsTracker.JobStatus.Running, JobsTracker.JobStatus.Ready) == JobsTracker.JobStatus.Running


def runJobWithSpecificHash(hash_value):
    maxMemoryMB = 96*1024 # MBytes
    maxPoints = 1e+11
    jobs = createFilesForSubmit(maxMemoryMB, maxPoints)
    jobs = sorted(jobs, key = lambda j : j.problem.Npoints)
    memoryResource.resource.value = maxMemoryMB


    j = jobs[0]
    j.bitsRepack.bits_per_sample = 16
    j.bitsRepack.dst_little_endian = False
    j.bitsRepack.src_little_endian = False
    j.problem.dim = 1
    j.problem.m_intervals_per_dim = 4096
    j.problem.n_cells_total = j.problem.m_intervals_per_dim ** j.problem.dim
    j.problem.Npoints = 40960
    j.problem.stride = 0
    j.rng.offset = 0
    j.subtask.cur_task = 0
    j.subtask.Ntasks = 1
    run(j)
    
    
    for j in jobs:
        if hash(j) == hash_value:
            run(j, write_results=False)    


def runAllJobs():
    maxMemoryMB = 96*1024 # MBytes
    maxPoints = 1e+11
    jobs = createFilesForSubmit(maxMemoryMB, maxPoints)
    jobs = sorted(jobs, key = lambda j : j.problem.Npoints)
    memoryResource.resource.value = maxMemoryMB

    
    assert(all(j.maxMemory() <= maxMemoryMB*1024**2 for j in jobs))
    print("Task with max memory consumption: %d MB" % (max(j.maxMemory() for j in jobs)/1024**2))

    print("Counting unfinished jobs...")       
    finished_jobs_hashes = jobsResults.loadFinishedJobsHashes()    
    jobsTracker.markAsFinishedByHash(finished_jobs_hashes)
    unfinished_jobs = []
    for j in tqdm.tqdm(jobs):
        if hash(j) not in finished_jobs_hashes:
            unfinished_jobs.append(j)
    unfinished_counter = len(unfinished_jobs)
    print(f"Yet unfinished jobs: {unfinished_counter}")

    t = time.time()
    unfinished_counter = len(unfinished_jobs)
    with multiprocessing.Pool(36) as pool:
        #for it in tqdm.tqdm(pool.map(run, unfinished_jobs, chunksize=1), total = len(jobs)):
        #for it in pool.map(run, unfinished_jobs, chunksize=1):
        for it in pool.imap_unordered(run, unfinished_jobs, chunksize=1):
            unfinished_counter -= 1            
            print(f"Yet unfinished jobs: {unfinished_counter}")

    # for it in tqdm.tqdm(map(run, unfinished_jobs), total = len(jobs)):
    #    pass

    print(f"Elapsed {time.time() - t} secs")
        
    assert(memoryResource.resource.value == maxMemoryMB)

    
    
if __name__ == '__main__':
    #runAllJobs()
    runJobWithSpecificHash(0x13d6cb948de589c3)
