import numpy as np
import scipy
from numba import jit

class Hypercube:
    def __init__(self, dim):
        self.dim = dim
        self.data = np.zeros(2**dim, dtype=np.int32)
        self.datasize = 2**dim

    def chain(self, index, bit):
        return (index << 1) % self.datasize + bit

    def fill(self, bits):
        _fill(self.data, self.dim, bits)

    def sampleInitialIndex(self):
        index = 0
        for n in range(10*self.datasize):
            index = self.chain(index, np.random.randint(0, 2, dtype=np.uint8))
        return index

    def chi2cdf(self):
        npoints = np.sum(self.data)
        mean = npoints/self.datasize
        chi2val = np.sum((self.data - mean)**2)/mean
        return scipy.stats.chi2(self.datasize-1).cdf(chi2val)

    def collapse(self, index):
        H = Hypercube(self.dim - 1)
        for n in range(self.datasize):
            newn = ((n >> (index+1)) << index) + (n % (2**index))
            H.data[newn] += self.data[n]
        return H

    def slice(self, index, value):
        H = Hypercube(self.dim - 1)
        for n in range(self.datasize):
            if ((n & (2**index)) >> index) == value:
                newn = ((n >> (index+1)) << index) + (n % (2**index))
                H.data[newn] += self.data[n]
        return H

    def groupTest(self):
        return grouptest(self.data)

    def H2(self):
        h2 = 0
        total = np.sum(self.data)
        for n in range(self.datasize):
            p = self.data[n]/total
            if p > 0:
                h2 -= p*np.log2(p)
        return h2/self.dim


def _group(expected, data, idx0, idx1):
    if idx1 < idx0:
        idx0, idx1 = idx1, idx0
    data2 = np.delete(data, idx1)
    expected2 = np.delete(expected, idx1)
    data2[idx0] += data[idx1]
    expected2[idx0] += expected[idx1]
    return data2, expected2


#@jit(nopython=False)
def _grouptest(data):
    total = np.sum(data)
    mean = total / len(data)
    expected = np.ones(len(data)) * mean

    for n in range(1, 2**len(data)-1): #we don't consider combinations when none or all elements are selected
        sum_actual = 0
        sum_expected = 0
        nnn = n
        for nn in range(len(data)):
            if nnn % 2 == 1:
                sum_actual += data[nn]
                sum_expected += expected[nn]
            nnn >>= 1

        p = sum_expected/total
        sigma = np.sqrt(p*(1-p)/total)
        deviations = (sum_actual - sum_expected)/sum_expected/sigma
        yield deviations

def grouptest(data):
    res = []
    for r in _grouptest(data):
        res.append(r)
    #res = sorted(np.unique(np.abs(res)), reverse=True)
    return res


@jit(nopython=True)
def _chain(datasize, index, bit):
    return (index << 1) % datasize + bit

@jit(nopython=True)
def _fill(data, dim, bits):
    index = 0

    for n in range(dim-1):
        index = _chain(len(data), index, bits[n])

    for n in range(dim-1, len(bits)):
        index = _chain(len(data), index, bits[n])
        data[index] += 1

