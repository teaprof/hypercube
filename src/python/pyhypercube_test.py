from hypercube import *

rng = RNG()
problem = HypercubeProblem()
tester = StatTest()
res = tester.run(rng, problem, 10)
print(res.chi2cdf)

