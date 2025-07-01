import sys, os
sys.path.append(os.path.abspath("../build/debug/python"))
sys.path.append(os.path.abspath("../../build/debug/python"))
import pyhypercube
from typing import List

class RNG:
    def __init__(self, rng_id = 0, offset = 0):
        rng_opts = pyhypercube.RandomNumberGeneratorDescription()
        rng_opts.offset = 0
        rng_opts.rng_id = 0
        self.rng = pyhypercube.createGenerator(rng_opts)
        
class HypercubeProblem:
    def __init__(self, dim = 2, m_intervals_per_dim = 10, Npoints = 10000):
        self.problem = pyhypercube.HypercubeProblem()
        self.problem.dim = dim
        self.problem.m_intervals_per_dim = m_intervals_per_dim
        self.problem.N = Npoints
        self.problem.n_cells_total = self.problem.m_intervals_per_dim ** self.problem.dim 
                
class HypercubeSampler:
    def __init__(self, problem: HypercubeProblem):
        self.hsampler = pyhypercube.HypercubeSampler(problem.problem)

class Subtask:
    def __init__(self, cur_task = 0, Ntasks = 1):
        self.subtask = pyhypercube.SubtaskParameters()
        self.subtask.cur_task = cur_task
        self.subtask.Ntasks = Ntasks
        
class Tester:
    def __init__(self):
        self.tester = pyhypercube.Chi2BasedTest1()
        
    def run(self, rng: RNG, problem: HypercubeProblem, nthreads = 1):
        subtask = Subtask(cur_task=0, Ntasks=1)
        sampler = HypercubeSampler(problem)
        subtask_results = self.tester.run(problem.problem, subtask.subtask, sampler.hsampler, rng.rng, nthreads)
        return self.collect(problem, 1, [subtask_results])       
        
    def runSubtask(self, rng: RNG, problem: HypercubeProblem, subtask: Subtask, nthreads = 1):
        sampler = HypercubeSampler(problem)
        res = self.tester.run(problem.problem, subtask.subtask, sampler.hsampler, rng.rng, nthreads)
        return res
    
    def collect(self, problem: HypercubeProblem, Nsubtasks: int, subtask_results: List[pyhypercube.SubtaskResults]):
        res = self.tester.collect(problem.problem, Nsubtasks, subtask_results)
        return res
        
        
        