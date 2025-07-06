import hashlib
import multiprocessing.managers
from operator import mul
from hypercube import *
import itertools, math
import tqdm
import multiprocessing, ctypes, contextlib
import time, csv, json, enum


def createFilesForSubmit(maxMemory):
    maxcells = maxMemory//4
    rng_type = [1]
    dimensions = [1, 2, 3, 4, 5, 6, 7, 8]
    m_intervals_per_dim = [10, 20, 100]
    n_points_per_cell = [10, 20, 50, 100, 1000, 10000, 100000]
    jobs = []
    for rng, dim, m, n_per_cell in itertools.product(rng_type, dimensions, m_intervals_per_dim, n_points_per_cell):
        rng = RNG(rng, 0)
        total_cells = m ** dim
        total_points = n_per_cell*total_cells
        problem = HypercubeProblem(dim, m, total_points)
        if total_cells < maxcells:
            nsubtasks = math.ceil(total_cells / maxcells)
            for cursubtask in range(nsubtasks):
                subtask = Subtask(cursubtask, nsubtasks)
                job = HypercubeJob(rng, problem, subtask)
                jobs.append(job)
        else:
            #print(total_cells*4)
            pass
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
    class JobStatus:
        Pending = 0
        Running = 1
        Ready = 2
        
    def __init__(self):
        self.mutex = multiprocessing.Lock()
        self.map = multiprocessing.Manager().dict()
        
    def add_if_not_exists(self, key, value: int = JobStatus.Pending):
        with self.mutex:
            if key not in self.map:
                self.map[key] = value
                return True
            else:
                return False
    
    def compare_and_swap(self, key, expected: int, newvalue: int):
        with self.mutex:
            assert key in self.map
            if self.map[key] == expected:
                self.map[key] = newvalue
                return expected
            else:
                return self.map[key]
            
    def save(self):
        with self.mutex:
            # create a local copy of dict from proxy object
            map = {}
            for key, value in self.map.items():
                map[key] = value
            # save local copy
            with open("jobstatus.json", "w") as f:
                json.dump(map, f)

    def load(self):
        with self.mutex:
            self.map.clear()
            try:
                with open("jobstatus.json", "r") as f:
                    map = json.load(f)                
                for key in map.keys():                    
                    if map[key] == JobsTracker.JobStatus.Running:
                        map[key] = JobsTracker.JobStatus.Pending
                for key, value in map.items():
                    ikey = int(key)
                    self.map[ikey] = value
            except FileNotFoundError:
                pass
        
            
class JobResults:
    def __init__(self):
        self.mutex = multiprocessing.Lock()
    
    def addResults(self, job, res):
        with self.mutex:            
            # create header if file not exists
            try:
                with open("results.csv", "x") as f:
                    f.write("rng offset dim m N Mtot curtask Ntasks sum sum2 chi2cdf\n")
            except FileExistsError:
                pass
            # write new data to the file
            with open("results.csv", "a") as f:
                f.write(f"{repr(job)} {res.sum} {res.sum2} {res.chi2cdf()}\n")
                
    # def getFinishedJobs(self):
    #     jobs = []
    #     with self.mutex:
    #         with open("results.csv", "r"):
    #             reader = csv.reader(f, delimiter=" ")
    #             for row in reader:
    #                 rng = RNG(row[0], row[1])
    #                 problem = HypercubeProblem(row[2], row[3], row[4])
    #                 subtask = Subtask(row[6], row[7])
    #                 jobs.append(HypercubeJob(rng, problem, subtask))
                
        
            

memoryResource = Resource(0)
jobsTracker = JobsTracker()
jobsResults = JobResults()


def run(job: HypercubeJob):    
    # check if the job is not done yet
    h = hash(job)
    jobsTracker.add_if_not_exists(h, JobsTracker.JobStatus.Pending)
    if jobsTracker.compare_and_swap(h, JobsTracker.JobStatus.Pending, JobsTracker.JobStatus.Running) != JobsTracker.JobStatus.Pending:
        print(f"Skipping job {h}")
        return
    
    # lock memory for the job
    global memoryResource    
    mem = math.ceil(job.maxMemory() / 1024**2) # MB
    with memoryResource.borrow(mem):
        #delay = min(10, job.problem.Npoints*1e-10 + 0.1)
        #print(f"job {h}: {job.problem.Npoints} -> {delay} secs, mem slots = {mem}")
        print(f"job {h}: {float(job.problem.Npoints):g} points, mem slots = {mem}")
        #time.sleep(delay)
        print(repr(job))
        res = job.run()
    
    jobsResults.addResults(job, res)
    
    # mark the job as finished
    assert jobsTracker.compare_and_swap(h, JobsTracker.JobStatus.Running, JobsTracker.JobStatus.Ready) == JobsTracker.JobStatus.Running
    
    
if __name__ == '__main__':
    maxMemory = 4*1024**3
    jobs = createFilesForSubmit(maxMemory)
    h = [hash(j) for j in jobs]
    #assert(len(h) == len(set(h)))
    memoryResource.resource.value = maxMemory//(1024**2)
    
    assert(all(j.maxMemory() < maxMemory for j in jobs))
    print(max(j.maxMemory() for j in jobs)/1024**2)
    
    jobsTracker.load()
    #print(len(jobsTracker.map.values()))
    
    t = time.time()
    with multiprocessing.Pool() as pool:
        for it in tqdm.tqdm(pool.imap(run, jobs), total = len(jobs)):
            #print(len(jobsTracker.map.values()))
            jobsTracker.save()

    #for it in tqdm.tqdm(map(run, jobs), total = len(jobs)):
    #    pass
        
    print(f"Elapsed {time.time() - t} secs")
    jobsTracker.save()
        
    assert(memoryResource.resource.value == maxMemory//(1024**2))