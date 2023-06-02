#ifndef RANDOMBITGENERATOR_H
#define RANDOMBITGENERATOR_H

#include"circularbuffer.h"

#include<vector>
#include<random>

class randombitgenerator
{
public:
    randombitgenerator();

    template<class Iterator>
    void genBits(const Iterator& begin, const Iterator& end)
    {
        using valuetype = std::iterator_traits<Iterator>::value_type;
        for(Iterator it = begin; it < end; it++)
        {
            *it = gen<valuetype>();
        }
    }

    template<class T>
    T gen()
    {
        T res = 0;
        for(size_t n = 0; n < (sizeof(T)-1)/sizeof(rngtype::result_type) + 1; n++)
        {
            res += rng();
            res <<= sizeof(rngtype::result_type);
        }
        return res;
    }
private:
    using rngtype = std::mt19937;
    rngtype rng;
};

class RandomNumberEngine
{
public:
    virtual double operator()() = 0;
    virtual size_t randi(size_t min, size_t max) = 0;
};

template<class stdRNG>
class stdRandomNumberEngine : public RandomNumberEngine
{
public:
    stdRandomNumberEngine(stdRNG& _rng) : rng(_rng) {}
    virtual double operator()()
    {
        std::uniform_real_distribution<double> u(0, 1);
        return u(rng);
    }
    virtual size_t randi(size_t min, size_t max)
    {
        std::uniform_int_distribution<size_t> u(min, max);
        return u(rng);
    }
private:
    stdRNG& rng;
};

template<class stdRNG>
class CaterpillarBitGenerator
{
public:
    CaterpillarBitGenerator(stdRNG& _rng) : rng(_rng), current(0), restbits(0)
    {
        typename stdRNG::result_type v = stdRNG::max();
        significantBitsOfCurrent = 0;
        while(v > 0)
        {
            significantBitsOfCurrent++;
            v >>= 1;
        }
        for(size_t n = 0; n < buf.size(); n++)
            next();
    }
    virtual double operator()()
    {
        std::uniform_real_distribution<double> u(0, 1);
        return u(rng);
    }
    template<class V>
    V bits(size_t len = sizeof(V)*8)
    {
        static_assert(sizeof(V)*8 <= buf.size(), "");
        V res = 0;
        for(size_t n = 0; n < len; n++)
        {
            res <<= 1;
            res += buf[n];
        }
        for(size_t n = 0; n < len; n++)
            next();
        return res;
    }
    void next()
    {
        if(restbits == 0)
        {
            restbits = significantBitsOfCurrent;
            current = rng();
            next();
        } else {
            restbits--;
            bool res = current % 2;
            current >>= 1;
            buf.shiftleft(res);

        }
    }
    virtual size_t randi(size_t min, size_t max)
    {
        std::uniform_int_distribution<size_t> u(min, max);
        return u(rng);
    }

private:
    stdRNG& rng;
    stdRNG::result_type current;
    size_t significantBitsOfCurrent;
    size_t restbits;
    CircularBuffer<bool, 64> buf;
};


#endif // RANDOMBITGENERATOR_H
