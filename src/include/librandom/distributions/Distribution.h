#ifndef __DISTRIBUTION_H__
#define __DISTRIBUTION_H__

#include<librandom/rng/dynamic/generators/RandomBitGenerator.h>

#include <cassert>

class Distribution {
    public:
        virtual ~Distribution() {}
        virtual uint64_t operator()(RandomBitGenerator& rng) = 0;
        virtual uint64_t max() const = 0;
        virtual double getProbability(uint64_t value, const RandomBitGenerator& rng) const = 0;
};

class UniformIntDistributionRough : public Distribution {
public:
    UniformIntDistributionRough(uint64_t max_value) : max_value_(max_value) {}
    uint64_t operator()(RandomBitGenerator &rng) override {
        assert(max_value_ <= rng.max());
        size_t noutcomes = rng.max() + 1;
        size_t nbins = max_value_ + 1;
        size_t counter = noutcomes / nbins;
        return rng() % (max_value_ + 1);
    }
    uint64_t max() const override {
        return max_value_;
    }
    double getProbability(uint64_t value, const RandomBitGenerator& rng) const override {
        size_t noutcomes = rng.max() + 1;
        size_t nbins = max_value_ + 1;
        size_t counter = noutcomes / nbins;
        if(value < noutcomes % nbins) {
            counter++;
        }
        return static_cast<double>(counter)/noutcomes;
    }
    void discard(uint64_t N, RandomBitGenerator &rng) {
        rng.discardN(N);
    }
private:
    //uint16_t bitsPerValue_;
    uint64_t max_value_;
};

#endif