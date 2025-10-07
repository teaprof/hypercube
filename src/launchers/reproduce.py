import numpy as np
import dataclasses
import scipy
from hypercube import *

@dataclasses.dataclass
class StatRes:
    sum: int
    sum2: int
    chi2: float
    chi2cdf: float


class HypercubeTest:
    def __init__(self, dim, m, N):
        self.dim = dim
        self.m = m
        self.N = N
        
    def run(self, rng):
        size = self.m ** self.dim
        data = np.zeros(size)
        for n in range(self.N):
            r = rng()
            idx = int(r) % int(size)
            data[idx] += 1
        chi2 = 0
        mean = self.N/size
        for v in data:
            chi2 += np.square(v - mean)/mean
        chi2 /= size
        chi2cdf = scipy.stats.chi2.cdf(chi2, size-1)
        res = StatRes(self.N, (data*data).sum(), chi2, chi2cdf)
        return res
                    

engine = pyhypercube.MT19937Wrapper()
h = HypercubeTest(1, 10, 100)
res = h.run(engine)
print(res)
