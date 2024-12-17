#include "RandomNumberWrapper.h"
#include <queue>

class IndexGenerator {
    public:
    IndexGenerator(size_t dim, size_t nIntervalsPerDim) : dim_(dim), nIntervalsPerDim_(nIntervalsPerDim) {}
    virtual size_t operator()(RandomBitGenerator& rng) = 0;
    protected:
    size_t dim_;
    size_t nIntervalsPerDim_;
    size_t sub2ind(const std::vector<size_t>& multiindex) {
        size_t idx = 0;
        assert(multiindex.size() == dim_);
        for(size_t k = 0; k < dim_; k++) {
            idx = nIntervalsPerDim_*idx + multiindex[k];
        }
        return idx;
    }
};

class BitindexGenerator : public IndexGenerator {
    public:
    BitindexGenerator(size_t dim, bool littleEndian) : IndexGenerator(dim, 2), littleEndian_(littleEndian) {}
    virtual size_t operator()(RandomBitGenerator& rng) {
        size_t idx = 0;
        for(size_t n = 0; n < dim_; n++)
            idx = (idx<<1) + pop_front(rng);
        return idx;
    }
    bool pop_front(RandomBitGenerator& rng) {
        if(bits.empty()) {
            size_t val = rng();
            if(littleEndian_) {
                //start adding from hi bit
                uint64_t high_mask = 1;
                for(size_t n = 1; n < rng.nbits(); n++) {
                    high_mask <<= 1;
                }
                for(size_t n = 0; n < rng.nbits(); n++) {
                    bits.push(val % high_mask != 0);
                    val <<= 1;
                }
            } else {
                //start adding from lo bit
                for(size_t n = 0; n < rng.nbits(); n++) {
                    bits.push(val % 2 != 0);
                    val >>= 1;
                }
            }
        }
        bool res =  bits.front();
        bits.pop();
        return res;
    };
    private:
    std::queue<bool> bits;
    bool littleEndian_;
};

/*class KIndependentGenerator : public IndexGenerator {
    public:
    KIndependentGenerator(size_t dim, size_t nIntervalsPerDim) : IndexGenerator(dim, nIntervalsPerDim) {}
    virtual size_t operator()(RandomBitGenerator& rng) {
        std::vector<size_t> multiindex(dim_, 0);
        for(size_t k = 0; k < dim_; k++)
            multiindex[k] = rng(nIntervalsPerDim_);
        size_t idx = sub2ind(multiindex);
        return idx;
    }
};*/

class ChainGenerator : public IndexGenerator {
    public:
    ChainGenerator(IntGenerator& gen, size_t dim, size_t nIntervalsPerDim, size_t stride) : gen_(gen), IndexGenerator(dim, nIntervalsPerDim), stride_(stride) {};
    virtual size_t operator()() {
        if(multiindex.empty())
            for(size_t n = 0; n < dim_; n++)
                multiindex.push_back(gen_(nIntervalsPerDim_));
        else {
            for(size_t n = 0; n < stride_; n++) {
                multiindex.push_back(gen_(nIntervalsPerDim_));
            }
            multiindex.erase(multiindex.begin(), multiindex.end() - dim_);
        }
        return sub2ind(multiindex);
    }
    private:
        IntGenerator& gen_;
        size_t stride_;
        std::vector<size_t> multiindex;
};
