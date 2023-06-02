#ifndef HYPERCUBETEST_H
#define HYPERCUBETEST_H

#include "randombitgenerator.h"

#include <stddef.h>
#include <random>
#include <boost/math/distributions/chi_squared.hpp>
#include <iostream>


template<class RandomNumberEngine>
class HypercubeTest
{
public:
    HypercubeTest(size_t _dim, size_t _nPoints, size_t _nIntervals) :
        dim(_dim), nPoints(_nPoints), nIntervals(_nIntervals) {}

    double operator()(RandomNumberEngine& rng)
    {
        size_t ncells = 1;
        for(size_t k = 0; k < dim; k++)
            ncells *= nIntervals;
        std::vector<uint64_t> buf(ncells, 0);
        for(size_t n = 0; n < nPoints; n++)
        {
            size_t idx = genIndex(rng);
            buf[idx]++;
        }
        uint64_t sum2 = 0;
        for(auto nn : buf)
        {
            uint64_t nn64 = nn;
            sum2 += nn64*nn64;
        }
        double mean = static_cast<double>(nPoints)/ncells;
        double chi2 = sum2/mean - nPoints;
        boost::math::chi_squared_distribution dist(ncells-1);
        return boost::math::cdf(dist, chi2);
    }


    virtual size_t genIndex(RandomNumberEngine& rng)
    {
        size_t idx = 0;
        for(size_t k = 0; k < dim; k++)
            idx = idx*nIntervals + rng.randi(0, nIntervals-1);
        return idx;
    }
    size_t dim, nPoints, nIntervals;
private:

};


template<class RandomBitGenerator>
class BinaryHypercubeTest : public HypercubeTest<RandomBitGenerator>
{
public:
    BinaryHypercubeTest(size_t dim, size_t nPoints) :
        HypercubeTest<RandomBitGenerator>(dim, nPoints, 2) {}

    virtual size_t genIndex(RandomBitGenerator& rng) override
    {
        return rng.template bits<size_t>(this->dim);
    }
private:
};

#endif // HYPERCUBETEST_H
