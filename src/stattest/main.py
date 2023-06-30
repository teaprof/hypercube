import time

import numpy as np
from numba import jit
import tqdm, scipy
from matplotlib import pyplot as plt

class Hypercube:
    def __init__(self, dim):
        self.dim = dim
        self.data = np.zeros(2**dim, dtype=np.int32)
        self.datasize = 2**dim

    def chain(self, index, bit):
        return (index << 1) % self.datasize + bit

    #def sample(self, index):


    def cdf(self):
        npoints = np.sum(self.data)
        mean = npoints/self.datasize
        chi2val = np.sum((self.data - mean)**2)/mean
        return scipy.stats.chi2(self.datasize-1).cdf(chi2val)

    def sampleInitialIndex(self):
        index = 0
        for n in range(10*self.datasize):
            index = self.chain(index, np.random.randint(0, 2, dtype=np.uint8))
        return index


if __name__ == "__main__":
    dim = 2
    h = Hypercube(dim)
    N = h.datasize*1000
    bits = np.zeros(N, dtype=np.bool_)
    t1 = time.perf_counter()
    corr = 0.48
    bit = 0
    for n in tqdm.tqdm(range(N)):
        if np.random.rand() < corr:
            bit = 1 - bit
        bits[n] = bit

    index = h.sampleInitialIndex()
    for n in tqdm.tqdm(range(N)):
        index = h.chain(index, bits[n])
        h.data[index] += 1

    h2 = Hypercube(1)
    h2.data[0] = h.data[0] + h.data[3]
    h2.data[1] = h.data[1] + h.data[2]
    print(f'Elapsed {time.perf_counter() - t1} secs')
    print('cdf = ', h.cdf())
    print('cdf2 = ', h2.cdf())
    acor = np.corrcoef(bits[1:], bits[:-1])[1, 0]
    sigma = 0.5/np.sqrt(N)
    print('autocorr = ', acor, ' = ', acor/sigma, ' sigma')


    xstr = []
    for n in range(h.datasize):
        xstr.append(format(n, f'0>{dim}b'))
    for x, y in zip(xstr, h.data):
        print(x, ':', y)
    #plt.figure()
    #plt.bar(xstr, h.data)
    #plt.bar(range(h.datasize), h.data)
    #plt.grid()
    #plt.show()
