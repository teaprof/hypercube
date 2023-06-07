#ifndef HYPERCUBETEST_H
#define HYPERCUBETEST_H

#include "randombitgenerator.h"

#include <stddef.h>
#include <random>
#include <boost/math/distributions/chi_squared.hpp>
#include <iostream>
#include "indexgenerator.h"


template<class rng>
class HypercubeTest
{
public:
    HypercubeTest(size_t _dim, size_t _nPoints, size_t _nIntervals) :
        dim(_dim), nPoints(_nPoints), nIntervals(_nIntervals) {}

    double operator()(rng& r, bool verbose = false)
    {
        size_t ncells = 1;
        for(size_t k = 0; k < dim; k++)
            ncells *= nIntervals;
        if(verbose)
            std::cout<<"ncells = "<<ncells<<std::endl;
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
        double dof = ncells-1;
        if(verbose)
        {
            std::cout<<"math expectation: "<<mean<<std::endl;
            std::cout<<"chi2: "<<chi2<<std::endl;
            std::cout<<"dof : "<<dof<<std::endl;
        }
        boost::math::chi_squared_distribution dist(dof);
        return boost::math::cdf(dist, chi2);
    }

    virtual size_t genIndex(rng &r) = 0;
    /*{
        size_t idx = 0;
        std::uniform_int_distribution<size_t> u(0, nIntervals-1);
        for(size_t k = 0; k < dim; k++)
            idx = idx*nIntervals + u(r);
        return idx;
    }*/
    size_t dim, nPoints, nIntervals;
private:

};

class HypercubeTest2 : public HypercubeTest<RandomBitGenerator>
{
public:
    HypercubeTest2(size_t _dim, size_t _nPoints, size_t _mIntervals, IndexGeneratorBase<RandomBitGenerator>& _indexGenerator) :
        HypercubeTest(_dim, _nPoints, _mIntervals),
        indexGenerator(_indexGenerator) {}

    virtual size_t genIndex(RandomBitGenerator& r) override
    {
        return indexGenerator.index(r);
    }
private:
    IndexGeneratorBase<RandomBitGenerator>& indexGenerator;
};


#endif // HYPERCUBETEST_H
