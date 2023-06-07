#ifndef INDEXGENERATOR_H
#define INDEXGENERATOR_H

#include <vector> //for size_t type
#include <cassert>
#include <cmath>
#include "randombitgenerator.h"

template<class rng>
class IndexGeneratorBase
{
public:
    virtual size_t index(rng& r) = 0;
};

class BinaryIndexGenerator : public IndexGeneratorBase<RandomBitGenerator>
{
public:
    BinaryIndexGenerator(size_t _dim, size_t _mIntervals, size_t _stride) :
        dim(_dim), mIntervals(_mIntervals), stride(_stride), _index(0), indexInitialized(false), restbits(0)
    {
        assert((mIntervals & (mIntervals-1)) == 0); // "mIntervals should be power of two"
        bitsperdim = std::log2(mIntervals);
        totalbits = bitsperdim*dim;
        assert(totalbits < sizeof(size_t)*8); // "too many cells in hypercube, increase dimension or mIntervals"
    }

    size_t index(RandomBitGenerator& r)
    {
        if(indexInitialized == false || stride == 0)
        {
            _index = gen(r, bitsperdim*dim);
            indexInitialized = true;
        } else {
            _index <<= stride;
            _index = _index - ((_index >> totalbits) << totalbits);
            size_t newbits = gen(r, stride);
            _index += newbits;
        }
        return _index;
    }

    size_t gen(RandomBitGenerator& r, size_t bits)
    {
        size_t res = 0;
        while(bits > 0)
        {
            if(restbits == 0)
            {
                current = r.genNative();
                restbits = r.nativebits;
            }
            size_t bb = bits < restbits? bits : restbits;
            if(bb > 0)
            {
                res <<= bb;
                res += current & ((static_cast<size_t>(1) << bb) - 1);
                bits -= bb;
                restbits -= bb;
                current >>= bb;
            }
        }
        return res;
    }

private:
    size_t dim, mIntervals, stride;
    size_t _index;
    size_t bitsperdim, totalbits;
    bool indexInitialized;

    size_t current;
    size_t restbits;
};



class FloatingPointIndexGenerator: public IndexGeneratorBase<RandomBitGenerator>
{
public:
    FloatingPointIndexGenerator(size_t _dim, size_t _mIntervals, size_t _stride, size_t _bytesPerFloat) :
        dim(_dim), mIntervals(_mIntervals), stride(_stride), bytesPerFloat(_bytesPerFloat)
    {
        skipBytes = 0;
        if(bytesPerFloat > sizeof(size_t))
        {
            skipBytes = bytesPerFloat - sizeof(size_t);
            bytesPerFloat = sizeof(size_t);
        }
        maxValue = (static_cast<size_t>(1) << 8*bytesPerFloat);
    }

    size_t index(RandomBitGenerator& r)
    {
        if(_index.empty())
        {
            _index.assign(dim, 0);
            for(auto &it: _index)
                it = floatToIndex(genfloat(r));
        } else {
            for(size_t n = 0; n < dim - stride; n++)
                _index[n] = _index[n+stride];
            for(size_t n = dim - stride; n < dim; n++)
                _index[n] = floatToIndex(genfloat(r));
        }
        return getlinearindex();
    }

    float genfloat(RandomBitGenerator& r)
    {
        size_t n = r.gen(bytesPerFloat*8);
        if(skipBytes > 0)
            r.gen(skipBytes);
        return static_cast<float>(n)/maxValue;
    }
    size_t floatToIndex(float v)
    {
        size_t idx = v*mIntervals;
        if(idx == mIntervals)
            idx = mIntervals - 1;
        return idx;
    }
    size_t getlinearindex()
    {
        size_t idx = 0;
        for(const auto& it: _index)
            idx = idx*mIntervals + it;
        return idx;
    }
private:
    size_t dim, mIntervals, stride, bytesPerFloat;
    size_t skipBytes, maxValue;
    std::vector<size_t> _index;
};



#endif // INDEXGENERATOR_H
