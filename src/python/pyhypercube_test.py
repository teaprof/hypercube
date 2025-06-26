import sys, os
sys.path.append(os.path.abspath("../build/debug/python"))
sys.path.append(os.path.abspath("../../build/debug/python"))
print(sys.path[-1])
import pyhypercube

rng_opts = pyhypercube.RandomNumberGeneratorDescription()
rng_opts.offset = 0
rng_opts.rng_id = 0

rng = pyhypercube.createGenerator(rng_opts)

problem = pyhypercube.HypercubeProblem()
problem.dim = 2
problem.m_intervals_per_dim = 10
problem.N = 10000
problem.n_cells_total = problem.m_intervals_per_dim ** problem.dim 

hsampler = pyhypercube.HypercubeSampler(problem)

subtask = pyhypercube.SubtaskParameters()
subtask.cur_task = 0
subtask.Ntasks = 1


tester = pyhypercube.Chi2BasedTest1()

pyhypercube.run(problem, subtask, sampler=hsampler)

res = tester.run(problem, subtask, hsampler, rng, 1)
res2 = tester.collect(problem, subtask.Ntasks, [res])
print(res2.chi2cdf)
