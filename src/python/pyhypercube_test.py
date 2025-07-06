from hypercube import *

rng = RNG()
problem = HypercubeProblem()
tester = StatTest()
res = tester.run(rng, None, problem, 10)
print(res.chi2cdf())

repack = BitsRepack(25, False, True)
res = tester.run(rng, repack, problem, 10)
print(res.chi2cdf())
print(res.sum, ":", res.sum2)

repack.bits_per_sample = None
rng1 = repack.rng(rng)
for n in range(10):
    print(rng1())
    