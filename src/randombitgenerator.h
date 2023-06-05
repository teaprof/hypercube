#ifndef RANDOMBITGENERATOR_H
#define RANDOMBITGENERATOR_H

#include"circularbuffer.h"

#include<vector>
#include<random>

class RandomBitGenerator
{
   public:
        template<class Iterator>
        void fill(const Iterator& begin, const Iterator& end);

        template<class T>
        T gen(size_t bits = sizeof(T)*8);

        template<class T>
        T randi(T min, T max);
};

template<class stdrngtype>
class StdRandomBitGenerator: public RandomBitGenerator
{
public:
    static_assert (stdrngtype::max() & (stdrngtype::max()+1) == 0, "stdrngtype::max should be 2^k-1");

    StdRandomBitGenerator(stdrngtype &_rng) : rng(_rng) {}

    template<class Iterator>
    void fill(const Iterator& begin, const Iterator& end)
    {
        using valuetype = typename std::iterator_traits<Iterator>::value_type;
        for(Iterator it = begin; it < end; it++)
        {
            *it = gen<valuetype>();
        }
    }

    template<class T>
    T gen(size_t bits)
    {
        constexpr size_t nbitssignificant = std::log2(stdrngtype::max()+1);
        T res = 0;
        for(size_t n = 0; n < (bits - 1)/nbitssignificant + 1; n++)
        {
            res += rng();
            res <<= nbitssignificant;
        }
        res = res & ((1<<bits) - 1);
        return res;
    }
private:
    stdrngtype rng;
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
        static_assert( sizeof(V)*8 <= (decltype(buf)::size()) , "");
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
    typename stdRNG::result_type current;
    size_t significantBitsOfCurrent;
    size_t restbits;
    CircularBuffer<bool, 64> buf;
};


#endif // RANDOMBITGENERATOR_H
