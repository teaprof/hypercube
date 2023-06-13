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

        size_t NPointsActual = 0;
        while(true)
        {
            try {
                size_t idx = genIndex(r);
                buf[idx]++;
            }  catch (const std::runtime_error& err) {
                if(verbose)
                    std::cout<<"File reading finished."<<std::endl;
                break;
            }

            NPointsActual++;
            if(nPoints > 0)
            {
                if(NPointsActual >= nPoints)
                    break;
            }
        }

        uint64_t sum2 = 0;
        for(auto nn : buf)
        {
            uint64_t nn64 = nn;
            sum2 += nn64*nn64;
        }
        double mean = static_cast<double>(NPointsActual)/ncells;
        double chi2 = sum2/mean - NPointsActual;
        double dof = ncells-1;
        if(verbose)
        {
            std::cout<<"NPoints actual: "<<NPointsActual<<std::endl;
            std::cout<<"math expectation: "<<mean<<std::endl;
            std::cout<<"chi2: "<<chi2<<std::endl;
            std::cout<<"dof : "<<dof<<std::endl;
        }
        boost::math::chi_squared_distribution dist(dof);
        return boost::math::cdf(dist, chi2);
    }

    virtual size_t genIndex(rng &r) = 0;
    /*{
        //thi implementation is for std::random, it can't be compiled for other kinds of rng
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

class HypercubeTestPy : public HypercubeTest2
{
public:
    HypercubeTestPy(size_t _dim, size_t _nPoints, size_t _mIntervals, IndexGeneratorBase<RandomBitGenerator>& _indexGenerator) :
        HypercubeTest2(_dim, _nPoints, _mIntervals, _indexGenerator) {};

    void push(const std::vector<char>& bytes);
};

#endif // HYPERCUBETEST_H
