import hashlib
import sys, os, math, pickle
#sys.path.append(os.path.abspath("./build/release/python"))
sys.path.append(os.path.abspath("../build/release/python"))
#sys.path.append(os.path.abspath("../../build/release/python"))
#sys.path.append(os.path.abspath("../build/debug/python"))
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
                
    #def rng(self):
    #    rng_opts = self.rngDescr()
    #    return pyhypercube.createGenerator(rng_opts)
    
    def __repr__(self):
        return f"{self.rng_id:4} {self.offset:4}"

    def __str__(self):
        return f"rngid = {self.rng_id:<4}, offset = {self.offset:<4}"

    def __eq__(self, other):
        return self.rng_id == other.rng_id and self.offset == other.offset

class BitsRepack:
    def __init__(self, bits_per_sample, src_little_endian=True, dst_little_endian=True):
        self.bits_per_sample = bits_per_sample
        self.src_little_endian = src_little_endian
        self.dst_little_endian = dst_little_endian
        
    def repackDescr(self):
        opts = pyhypercube.BitsRepackDescription()
        opts.bits_per_sample = self.bits_per_sample
        if opts.bits_per_sample == 0:
            opts.bits_per_sample = None
        opts.src_little_endian = self.src_little_endian
        opts.dst_little_endian = self.dst_little_endian
        return opts
    
    #def rng(self, source_rng: RNG):
    #    rng_opts = source_rng.rngDescr()
    #    repack_opts = self.repackDescr()            
    #    return pyhypercube.createGenerator(rng_opts, repack_opts)
    
    def __eq__(self, other):
        return self.bits_per_sample == other.bits_per_sample and self.src_little_endian == other.src_little_endian and self.dst_little_endian == other.dst_little_endian

    #def __repr__(self):
    #    return f"{self.bits_per_sample:4} {self.src_little_endian:4} {self.dst_little_endian:4}"

    @staticmethod        
    def toRepr(obj: Optional["BitsRepack"]):
        bits_per_sample = 0
        src_little_endian = True
        dst_little_endian = True        
        if obj:
            bits_per_sample = obj.bits_per_sample
            src_little_endian = obj.src_little_endian
            dst_little_endian = obj.dst_little_endian
        return f"{bits_per_sample:4} {src_little_endian:4} {dst_little_endian:4}"

    @staticmethod
    def toStr(obj: Optional["BitsRepack"]):
        if obj:
            bits_per_sample = obj.bits_per_sample
            src_little_endian = obj.src_little_endian
            dst_little_endian = obj.dst_little_endian
            return f"bps = {bits_per_sample:<4}, src_little_endian = {src_little_endian:<4}, dst_little_endian = {dst_little_endian:<4}"
        return f"no bits repack"
        

class HypercubeProblem:
    bytes_per_cell = 8
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

    def __eq__(self, other):
        return self.dim == other.dim and self.m_intervals_per_dim == other.m_intervals_per_dim and self.Npoints == other.Npoints and self.stride == other.stride
    
    
    def maxMemory(self):
        return HypercubeProblem.bytes_per_cell*self.n_cells_total #in bytes
    
    def __repr__(self):
        return f"{self.dim:4} {self.m_intervals_per_dim:10} {self.stride:6} {self.Npoints:10} {self.n_cells_total:10}"

    def __str__(self):
        return f"dim = {self.dim:<4}, m = {self.m_intervals_per_dim:<10}, stride = {self.stride:<6}, N = {self.Npoints:<8.2e}, n_cells_total = {self.n_cells_total:<8.2e}"

class HypercubeSampler:
    def __init__(self, problem: HypercubeProblem):
        self.problem = problem

    def __eq__(self, other):
        return self.sampler == other.sampler
        
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
    
    def __eq__(self, other):
        return self.cur_task == other.cur_task and self.Ntasks == other.Ntasks
    
    def __repr__(self):
        return f"{self.cur_task:4} {self.Ntasks:4}"

    def __str__(self):
        return f"cur_task = {self.cur_task:<4}, Ntasks = {self.Ntasks:<4}"

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
        rng_obj = self.createRng(rng, repack)         
        
        res = tester.run(problem.problem(), subtask.subtask(), sampler.sampler(), rng_obj, nthreads)
        # dummy results:
        #res = pyhypercube.SubtaskResults()
        #res.dof = 1
        #res.sum = 20
        #res.sum2 = 200
        return res

    def runSubtaskSingleThreaded(self, rng: RNG, repack: Optional[BitsRepack], problem: HypercubeProblem, subtask: Subtask):
        tester = pyhypercube.Chi2BasedTestSingleThreaded1()
        sampler = HypercubeSampler(problem)
        rng_obj = self.createRng(rng, repack)
        
        nthreads = 1
        res = tester.run(problem.problem(), subtask.subtask(), sampler.sampler(), rng_obj, nthreads)
        return res

    def collect(self, problem: HypercubeProblem, Nsubtasks: int, subtask_results: List[pyhypercube.SubtaskResults]):
        tester = pyhypercube.Chi2BasedTest1()
        res = tester.collect(problem.problem(), Nsubtasks, subtask_results)
        return res

    def createRng(self, rng: RNG, repack: Optional[BitsRepack]):
        rng_descr = rng.rngDescr()
        rng_archive_descr = pyhypercube.RNGArchiveDescription()
        rng_archive_descr.archive_file_name = "rng.bin"       
        #rng_archive_descr = None
         
        if rng_archive_descr is not None:
            if repack is not None:
                repack_descr = repack.repackDescr()
                rng_obj = pyhypercube.createGenerator(rng_descr, repack_descr, rng_archive_descr)
            else:
                rng_obj = pyhypercube.createGenerator(rng_descr, rng_archive_descr)
        else:
            if repack is not None:
                repack_descr = repack.repackDescr()
                rng_obj = pyhypercube.createGenerator(rng_descr, repack_descr)
            else:
                rng_obj = pyhypercube.createGenerator(rng_descr)
        return rng_obj

        
        
class HypercubeJob:
    def __init__(self, rng: RNG, bitsRepack: Optional[BitsRepack], problem: HypercubeProblem, subtask: Subtask):
        self.rng = rng
        self.bitsRepack = bitsRepack
        self.problem = problem
        self.subtask = subtask    
        
    def run(self):
        test = StatTest()
        points_per_thread = int(1e+9)
        # perform rounding up division
        nthreads = (self.problem.Npoints + points_per_thread - 1) // points_per_thread
        nthreads = min(nthreads, 20)
        res = test.runSubtask(self.rng, self.bitsRepack, self.problem, self.subtask, nthreads)
        return res

    def runSingleThreaded(self):
        test = StatTest()
        res = test.runSubtaskSingleThreaded(self.rng, self.bitsRepack, self.problem, self.subtask)
        return res

    def maxMemory(self):
        return self.problem.maxMemory()    
                
    def __repr__(self):
        return repr(self.rng) + " " + BitsRepack.toRepr(self.bitsRepack) + " " + repr(self.problem) + " " + repr(self.subtask)

    def __str__(self):
        return str(self.rng) + ", " + BitsRepack.toStr(self.bitsRepack) + ", " + str(self.problem) + ", " + str(self.subtask)

    def __eq__(self, other):
        return self.rng == other.rng and self.bitsRepack == other.bitsRepack and self.problem == other.problem and self.subtask == other.subtask
       
    def __hash__(self):
        h = hashlib.blake2b(digest_size=8)
        h.update(repr(self).encode())
        return int.from_bytes(h.digest(), 'big')
