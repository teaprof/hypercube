#ifndef INDEXGENERATOR_H
#define INDEXGENERATOR_H

class IndexGeneratorBase
{
public:
    virtual size_t index() = 0;
};

class BinaryIndexGenerator : public IndexGeneratorBase
{
public:
    BinaryIndexGenerator(size_t _dim, size_t _mIntervals, size_t _stride) :
        dim(_dim), mIntervals(_mIntervals), stride(_stride), _index(0), indexInitialized(false)
    {
        assert(mIntervals & (mIntervals-1) == 0); // "mIntervals should be power of two"
        bitsperdim = std::log2(mIntervals);
        totalbits = bitsperdim*dim;
        assert(totalbits < sizeof(size_t)*8); // "too many cells in hypercube, increase dimension or mIntervals"
    }

    template<class rng>
    size_t index(rng& r)
    {
        if(indexInitialized == false || stride == 0)
        {
            _index = r.template gen<size_t>(bitsperdim*dim);
            indexInitialized = true;
        } else {
            _index <<= stride;
            _index = _index - ((_index >> totalbits) << totalbits);
            size_t newbits = r.gen<size_t>(stride);
            _index += newbits;
        }
        return _index;
    }

private:
    size_t dim, mIntervals, stride;
    size_t _index;
    size_t bitsperdim, totalbits;
    bool indexInitialized;
};



class FloatingPointIndexGenerator: public IndexGeneratorBase
{
public:
    FloatingPointIndexGenerator(size_t _dim, size_t _mIntervals, size_t _stride, size_t _bytesPerFloat) :
        rng(_rng), dim(_dim), mIntervals(_mIntervals), stride(_stride), bytesPerFloat(_bytesPerFloat)
    {
        skipBytes = 0;
        if(bytesPerFloat > sizeof(size_t))
        {
            skipBytes = bytesPerFloat - sizeof(size_t);
            bytesPerFloat = sizeof(size_t);
        }
        maxValue = (1 << 8*bytesPerFloat);
    }

    template<class rng>
    size_t index(rng& r)
    {
        if(_index.empty())
        {
            _index.assign(dim, 0);
            for(auto &it: _index)
                it = genfloat(r);
        } else {
            for(size_t n = 0; n < dim - stride; n++)
                _index[n] = _index[n+stride];
            for(size_t n = dim - stride; n < dim; n++)
                _index[n] = genfloat(r);
        }
        return getlinearindex();
    }

    template<class rng>
    float genfloat(rng& r)
    {
        size_t n = r.gen<size_t>(bytesPerFloat*8);
        if(skipBytes > 0)
            r.gen<size_t>(skipBytes);
        return static_cast<float>(n)/maxValue;
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
