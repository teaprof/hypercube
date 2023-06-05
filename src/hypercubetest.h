#ifndef HYPERCUBETEST_H
#define HYPERCUBETEST_H

#include "randombitgenerator.h"

#include <stddef.h>
#include <random>
#include <boost/math/distributions/chi_squared.hpp>
#include <iostream>
#include "indexgenerator.h"


class HypercubeTest
{
public:
    HypercubeTest(size_t _dim, size_t _nPoints, size_t _nIntervals) :
        dim(_dim), nPoints(_nPoints), nIntervals(_nIntervals) {}

    template<class rng>
    double operator()(rng& r)
    {
        size_t ncells = 1;
        for(size_t k = 0; k < dim; k++)
            ncells *= nIntervals;
        std::vector<uint64_t> buf(ncells, 0);
        for(size_t n = 0; n < nPoints; n++)
        {
            size_t idx = genIndex(r);
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

    template<class rng>
    virtual size_t genIndex(rng &r)
    {
        size_t idx = 0;
        for(size_t k = 0; k < dim; k++)
            idx = idx*nIntervals + r.randi(0, nIntervals-1);
        return idx;
    }
    size_t dim, nPoints, nIntervals;
private:

};

class HypercubeTest2 : public HypercubeTest<RandomNumberEngine>
{
public:
    HypercubeTest2(size_t _dim, size_t _nPoints, size_t _mIntervals, IndexGeneratorBase& _indexGenerator) :
        HypercubeTest(_dim, _nPoints, _mIntervals),
        indexGenerator(_indexGenerator) {}

    template<class rng>
    virtual size_t genIndex(rng& r)
    {
        return indexGenerator.index(r);
    }
private:
    IndexGeneratorBase& indexGenerator;
};


#endif // HYPERCUBETEST_H
