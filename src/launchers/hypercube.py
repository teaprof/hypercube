import hashlib
import sys, os, math, pickle
sys.path.append(os.path.abspath("./build/debug/python"))
sys.path.append(os.path.abspath("../build/debug/python"))
sys.path.append(os.path.abspath("../../build/debug/python"))
import pyhypercube
from typing import List, Optional

class RNG:
    def __init__(self, rng_id = 0, offset = 0):
        self.rng_id = rng_id
        self.offset = offset
        
    def rngDescr(self):
        rng_opts = pyhypercube.RandomNumberGeneratorDescription()
        rng_opts.offset = self.offset
        rng_opts.rng_id = self.rng_id
        return rng_opts
                
    def rng(self):
        rng_opts = self.rngDescr()
        return pyhypercube.createGenerator(rng_opts)
    
    def __repr__(self):
        return f"{self.rng_id:4} {self.offset:4}"

class BitsRepack:
    def __init__(self, bits_per_sample, src_little_endian=True, dst_little_endian=True):
        self.bits_per_sample = bits_per_sample
        self.src_little_endian = src_little_endian
        self.dst_little_endian = dst_little_endian
        
    def repackOpts(self):
        opts = pyhypercube.BitsRepackDescription()
        opts.bits_per_sample = self.bits_per_sample
        if opts.bits_per_sample == 0:
            opts.bits_per_sample = None
        opts.src_little_endian = self.src_little_endian
        opts.dst_little_endian = self.dst_little_endian
        return opts
    
    def rng(self, source_rng: RNG):
        rng_opts = source_rng.rngDescr()
        repack_opts = self.repackOpts()            
        return pyhypercube.createGenerator(rng_opts, repack_opts)

    #def __repr__(self):
    #    return f"{self.bits_per_sample:4} {self.src_little_endian:4} {self.dst_little_endian:4}"
    
    @staticmethod
    def tostr(obj: Optional["BitsRepack"]):
        bits_per_sample = 0
        src_little_endian = True
        dst_little_endian = True        
        if obj:
            bits_per_sample = obj.bits_per_sample
            src_little_endian = obj.src_little_endian
            dst_little_endiad = obj.dst_little_endian
        return f"{bits_per_sample:4} {src_little_endian:4} {dst_little_endian:4}"
        
class HypercubeProblem:
    def __init__(self, dim = 2, m_intervals_per_dim = 10, Npoints = 10000, stride = None):
        self.dim = dim
        self.m_intervals_per_dim = m_intervals_per_dim
        self.Npoints = Npoints
        self.n_cells_total = m_intervals_per_dim ** dim
        if stride == None:
            stride = self.dim
        self.stride = stride
    
    def problem(self):
        p = pyhypercube.HypercubeProblem()
        p.dim = self.dim
        p.m_intervals_per_dim = self.m_intervals_per_dim
        p.N = self.Npoints
        p.n_cells_total = self.n_cells_total
        p.stride = self.stride
        return p
    
    def maxMemory(self):
        return 4*self.n_cells_total #in bytes
    
    def __repr__(self):
        return f"{self.dim:4} {self.m_intervals_per_dim:10} {self.stride:6} {self.Npoints:10} {self.n_cells_total:10}"
                
class HypercubeSampler:
    def __init__(self, problem: HypercubeProblem):
        self.problem = problem
        
    def sampler(self):
        return pyhypercube.HypercubeSampler(self.problem.problem())

class Subtask:
    def __init__(self, cur_task = 0, Ntasks = 1):
        self.cur_task = cur_task
        self.Ntasks = Ntasks
        
    def subtask(self):
        s = pyhypercube.SubtaskParameters()
        s.cur_task = self.cur_task
        s.Ntasks = self.Ntasks
        return s
    
    def __repr__(self):
        return f"{self.cur_task:4} {self.Ntasks:4}"
        
class StatTest:
    def __init__(self):
        pass
        
    def run(self, rng: RNG, repack: Optional[BitsRepack], problem: HypercubeProblem, nthreads = 1):
        tester = pyhypercube.Chi2BasedTest1()
        subtask = Subtask(cur_task=0, Ntasks=1)
        sampler = HypercubeSampler(problem)
        if repack is not None:
            rng_obj = repack.rng(rng)
        else:
            rng_obj = rng.rng()
        subtask_results = tester.run(problem.problem(), subtask.subtask(), sampler.sampler(), rng_obj, nthreads)
        return self.collect(problem, 1, [subtask_results])       
        
    def runSubtask(self, rng: RNG, repack: Optional[BitsRepack], problem: HypercubeProblem, subtask: Subtask, nthreads = 1):
        tester = pyhypercube.Chi2BasedTest1()
        sampler = HypercubeSampler(problem)
        if repack is not None:
            rng_obj = repack.rng(rng)
        else:
            rng_obj = rng.rng()
        res = tester.run(problem.problem(), subtask.subtask(), sampler.sampler(), rng_obj, nthreads)
        # dummy results:
        #res = pyhypercube.SubtaskResults()
        #res.dof = 1
        #res.sum = 20
        #res.sum2 = 200
        return res
    
    def collect(self, problem: HypercubeProblem, Nsubtasks: int, subtask_results: List[pyhypercube.SubtaskResults]):
        tester = pyhypercube.Chi2BasedTest1()
        res = tester.collect(problem.problem(), Nsubtasks, subtask_results)
        return res
        
        
class HypercubeJob:
    def __init__(self, rng: RNG, bitsRepack: Optional[BitsRepack], problem: HypercubeProblem, subtask: Subtask):
        self.rng = rng
        self.bitsRepack = bitsRepack
        self.problem = problem
        self.subtask = subtask    
        
    def run(self):
        test = StatTest()
        res = test.runSubtask(self.rng, self.bitsRepack, self.problem, self.subtask)
        return res
    
    def maxMemory(self):
        return self.problem.maxMemory()    
    
    def condorSubmitDescription(self, data_filename):
        mem = self.problem.maxMemory()
        mem = math.ceil(mem/1024/1024) + 50
        mem = f"{mem}M"
        return {
            "Executable": "/bin/python3",  # Or your Python script
            "Arguments": f"hypercube_job.py {data_filename}",
            "request_cpus": "1",
            "request_memory": mem,
            #"request_disk": "1024K",
            "Output": "job.out",
            "Error": "job.err",
            "Log": "job.log",
            "Queue": "1"}
    
    def printCondorSubmit(self, submit_filename, data_filename):
        descr = self.condorSubmitDescription(data_filename)
        with open(submit_filename, "w") as f:
            for key, value in descr.items():
                if key != "Queue":
                    f.write(f"{key} = {value}\n")
                else:
                    f.write(f"{key} {value}\n")
        with open(data_filename, "wb") as f:
            pickle.dump(self, f)
            
    def __repr__(self):
        return repr(self.rng) + " " + BitsRepack.tostr(self.bitsRepack) + " " + repr(self.problem) + " " + repr(self.subtask)
       
    def __hash__(self):
        h = hashlib.blake2b(digest_size=8)
        h.update(repr(self).encode())
        return int.from_bytes(h.digest(), 'big')
