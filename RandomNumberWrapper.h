#ifndef RANDOM_NUMBER_WRAPPER_H
#define RANDOM_NUMBER_WRAPPER_H

#include<random>
#include<vector>
#include<cassert>

class RandomNumberWrapper {
public:
    virtual size_t operator()(size_t maxval) = 0;
    virtual void discard() = 0;
    virtual void discardN(size_t n) {
        for(size_t i = 0; i < n; i++)
            discard();
    };
};

template<class Rng> 
class RandomNumberWrapperStd : public RandomNumberWrapper {
public:
    Rng rng;
    virtual size_t operator()(size_t maxval) {
        return rng() % maxval;
    }    
    virtual void discard() {
        rng();
    }
};

class MultiindexGenerator {
    public:
    virtual size_t operator()(RandomNumberWrapper& rng) = 0;
};

class IndexGenerator {
    public:
    virtual size_t operator()(RandomNumberWrapper& rng) {

    };
}

class ChainGenerator : public MultiindexGenerator {
    public:
    ChainGenerator(size_t dim, size_t shift, size_t maxval) : dim_(dim), shift_(shift), maxval_(maxval) {

    }
    private:
    size_t dim_;
    size_t shift_;
    size_t maxval_;
};

class KIndependentGenerator : public MultiindexGenerator {
    public:
    KIndependentGenerator(size_t dim, size_t maxval) : dim_(dim), maxval_(maxval) {}
    virtual size_t operator()(RandomNumberWrapper& rng) {
        std::vector<size_t> multiindex(dim_, 0);
        for(size_t k = 0; k < dim_; k++)
            multiindex[k] = rng(maxval_);
        size_t idx = sub2ind(multiindex);
        return idx;
    }
    size_t sub2ind(const std::vector<size_t>& multiindex) {
        size_t idx = 0;
        assert(multiindex.size() == dim_);
        for(size_t k = 0; k < dim_; k++) {
            idx = maxval_*idx + multiindex[k];
        }
        return idx;
    }
    private:
    size_t dim_;
    size_t maxval_;
};


using MT19937Wrapper = RandomNumberWrapperStd<std::mt19937>;

#endif