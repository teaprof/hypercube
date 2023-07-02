from hypercube import Hypercube
import time
from itertools import combinations

import numpy as np
from numba import jit
import tqdm, scipy
from matplotlib import pyplot as plt


def runTest(dim, mean, corr = 0.5):
    H = Hypercube(dim)
    N = H.datasize * mean
    bits = np.zeros(N, dtype=np.bool_)
    bit = 0
    for n in range(N):
        if np.random.rand() < corr:
            bit = 1 - bit
        bits[n] = bit
    index = H.sampleInitialIndex()
    for n in range(N):
        index = H.chain(index, bits[n])
        H.data[index] += 1
    deviations = H.groupTest()
    return deviations


def collectStatistics(N, dim, mean, corr = 0.5):
    res = None
    for n in tqdm.tqdm(range(N)):
        deviations = runTest(dim, mean, corr)
        if res is None:
            res = np.zeros((N, len(deviations)))
        res[n, :] = np.sort(deviations)
    return res

def plotecdf(x, ecdf):
    collection = []
    for it in ecdf:
        f = scipy.interpolate.interp1d(it.cdf.probabilities, it.cdf.quantiles)
        y1 = f(0.3)
        y2 = f(0.7)
        print(y1, y2)
        r = plt.Rectangle((x, y1), 0.2, y2-y1, color='red')
        collection.append(r)
    for it in collection:
        plt.gca().add_artist(it)


if __name__ == "__main__":
    dim = 2
    mean = 100
    NSamples = 1000
    res = collectStatistics(NSamples, dim, mean)
    ecdf0 = []
    for n in range(res.shape[1]):
        ecdf0.append(scipy.stats.ecdf(res[:, n]))

    res = collectStatistics(NSamples, dim, mean, 0.3)
    ecdf1 = []
    for n in range(res.shape[1]):
        ecdf1.append(scipy.stats.ecdf(res[:, n]))

    #plt.figure()
    #plotecdf(n, ecdf0)
    #plt.autoscale(enable=True)
    #plt.grid()
    #plt.show()

    #acor = np.corrcoef(bits[1:], bits[:-1])[1, 0]
    #sigma = 0.5/np.sqrt(N)
    #print('autocorr = ', acor, ' = ', acor/sigma, ' sigma')

