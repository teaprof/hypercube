import sys, os
sys.path.append(os.path.abspath("../build/debug/python"))
sys.path.append(os.path.abspath("../../build/debug/python"))
print(sys.path[-1])
import pyhypercube

rng_opts = pyhypercube.RandomNumberGeneratorDescription()

rng = pyhypercube.createGenerator(rng_opts)

problem = pyhypercube.HypercubeProblem()
sampler = pyhypercube.HypercubeSampler(problem)

subtask = pyhypercube.SubtaskParameters()

res = sampler(rng)


tester = pyhypercube.Chi2BasedTest1()

pyhypercube.run(problem, subtask, sampler)

res = tester.run(problem, subtask, sampler, rng, 1)

pass