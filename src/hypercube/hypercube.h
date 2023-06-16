#ifndef HYPERCUBETEST_H
#define HYPERCUBETEST_H

#include "randombitgenerator.h"

#include <stddef.h>
#include <random>
#include <boost/math/distributions/chi_squared.hpp>
#include <iostream>
#include "indexgenerator.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;


template<class rng>
class HypercubeTest
{
public:
    HypercubeTest(size_t _dim, size_t _nIntervals) :
        dim(_dim), nPoints(0), nIntervals(_nIntervals)
    {
        size_t ncells = 1;
        for(size_t k = 0; k < dim; k++)
            ncells *= nIntervals;
        buf.assign(ncells, 0);
    }

    double operator()(size_t _nPoints, rng& r, bool verbose = false)
    {
        if(verbose)
            std::cout<<"ncells = "<<buf.size()<<std::endl;

        size_t NPointsActual = 0;
        while(true)
        {
            try {
                size_t idx = genIndex(r);
                buf[idx]++;
            } catch (const EndOfBufferException) {
                break;
            }

            NPointsActual++;
            if(_nPoints > 0)
            {
                if(NPointsActual >= _nPoints)
                    break;
            }
        }
        nPoints += NPointsActual;

        uint64_t sum2 = 0;
        for(auto nn : buf)
        {
            uint64_t nn64 = nn;
            sum2 += nn64*nn64;
        }
        double mean = static_cast<double>(NPointsActual)/buf.size();
        double chi2 = sum2/mean - NPointsActual;
        double dof = buf.size()-1;
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

    size_t* data()
    {
        return buf.data();
    }
    size_t size()
    {
        return buf.size();
    }

private:
    std::vector<size_t> buf;
};

class HypercubeStrategies : public HypercubeTest<RandomBitGenerator>
{
public:
    HypercubeStrategies(size_t _dim, size_t _mIntervals, std::shared_ptr<IndexGeneratorBase<RandomBitGenerator>> _indexGenerator = nullptr) :
        HypercubeTest(_dim, _mIntervals), indexGenerator(_indexGenerator)
    {}

    void setIndexGenerator(bool binary, size_t stride, size_t bytesPerFloat)
    {
        if(binary)
            indexGenerator = std::make_shared<BinaryIndexGenerator>(dim, nIntervals, stride);
        else
            indexGenerator = std::make_shared<FloatingPointIndexGenerator>(dim, nIntervals, stride, bytesPerFloat);
    }

    virtual size_t genIndex(RandomBitGenerator& r) override
    {
        if(indexGenerator == nullptr)
            throw std::runtime_error("indexGenerator is nullptr, please, call ::setIndexGenerator");
        return indexGenerator->index(r);
    }
private:
    std::shared_ptr<IndexGeneratorBase<RandomBitGenerator>> indexGenerator;
};

class HypercubeTestPy : public HypercubeStrategies
{
public:
    HypercubeTestPy(size_t _dim, size_t _mIntervals) :
        HypercubeStrategies(_dim, _mIntervals, nullptr) {}

    double run(const char* buf, size_t size, bool verbose = false)
    {
        BufferedGenerator g;
        g.acceptbuffer(buf, size);        
        return (*this)(0, g, verbose);
    }
};

#endif // HYPERCUBETEST_H
