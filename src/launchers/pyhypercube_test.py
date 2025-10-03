from hypercube import *

rng = RNG()
problem = HypercubeProblem(1, 10, 1000)
tester = StatTest()
res = tester.run(rng, None, problem, 10)
print(res.chi2cdf())

repack = BitsRepack(25, False, True)
res = tester.run(rng, repack, problem, 10)
print(res.chi2cdf())
print(res.sum, ":", res.sum2)

print("With dummy repack")
data1 = []
repack.bits_per_sample = None
repack.src_little_endian = True
repack.dst_little_endian = True
rng1 = repack.rng(rng)
for n in range(10):
    data1.append(rng1())
print(data1)

print("Without repack")
data2 = []
rng2 = rng.rng()
for n in range(10):
    data2.append(rng2())
print(data2)

assert(data1 == data2)
            