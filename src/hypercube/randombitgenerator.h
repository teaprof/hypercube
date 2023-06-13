#ifndef RANDOMBITGENERATOR_H
#define RANDOMBITGENERATOR_H

#include"circularbuffer.h"

#include<vector>
#include<array>
#include<random>
#include<iostream>
#include<fstream>
#include<stdexcept>

class RandomBitGenerator
{
public:
    RandomBitGenerator(bool _verbose = false) : count(0), verbose(_verbose), nativebits(0) {};
    ~RandomBitGenerator()
    {
        if(verbose)
            std::cout<<"rng counter: "<<count<<std::endl;
    }
    virtual size_t gen(size_t bits = sizeof(size_t)*8)
    {
        size_t res = 0;
        for(size_t n = 0; n < (bits - 1)/nativebits + 1; n++)
        {
            res <<= nativebits;
            res += genNative();
        }
        size_t mask = (static_cast<size_t>(1)<<bits) - 1;
        res = res & mask;
        return res;
    }
    virtual size_t genNative() = 0;
    size_t count;
    size_t nativebits;
    bool verbose;
};

template<class stdrngtype>
class StdRandomBitGenerator: public RandomBitGenerator
{
public:
    static_assert ((stdrngtype::max() & (stdrngtype::max()+1)) == 0, "stdrngtype::max should be 2^k-1");

    StdRandomBitGenerator(stdrngtype &_rng, bool verbose = false) : RandomBitGenerator(verbose), rng(_rng)
    {
        nativebits = std::log2(stdrngtype::max());
    }

    virtual size_t genNative() override
    {
        count++;
        return rng();
    }
private:
    stdrngtype rng;
};

class FileBitGenerator: public RandomBitGenerator
{
public:
    FileBitGenerator(const std::string& filename, bool verbose = false) : RandomBitGenerator(verbose), f(filename, std::ios::in),
        bufRest(0), current(0)
    {
        nativebits = 8;
    }

    virtual size_t genNative() override
    {
        if(bufRest == 0 || current >= bufRest)
        {
            f.read(buf.data(), bufSize);
            bufRest = f.gcount();
            if(bufRest == 0)
                throw std::runtime_error("filesize too small for this test");
            current = 0;
        };
        return buf[current++];
    }
private:
    constexpr static size_t bufSize = 1024;
    std::array<char, bufSize> buf;
    size_t current, bufRest;
    std::ifstream f;
};

class VectorBitGenerator: public RandomBitGenerator
{
public:
    VectorBitGenerator(bool verbose = false) : RandomBitGenerator(verbose) {};

    void acceptbuffer(const char* buf, size_t size)
    {
        p = buf;
        q = p + size;
    }

    virtual size_t genNative() override
    {
        if(p < q)
            return *p++;
    }
private:
    const char *p, *q;
};



#endif // RANDOMBITGENERATOR_H
