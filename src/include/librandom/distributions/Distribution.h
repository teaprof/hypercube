#ifndef __DISTRIBUTION_H__
#define __DISTRIBUTION_H__

#include<librandom/rng/dynamic/generators/RandomBitGenerator.h>

#include <cassert>

class Distribution {
    public:
        virtual uint64_t operator()(RandomBitGenerator& rng) = 0;
        virtual uint64_t max() = 0;
};

class UniformIntDistributionRough : public Distribution {
public:
    UniformIntDistributionRough(uint64_t max_value) : max_value_(max_value) {}
    uint64_t operator()(RandomBitGenerator &rng) {
        assert(max_value_ <= rng.max());
        return rng() % (max_value_ + 1);
    }
    uint64_t max() {
        return max_value_;
    }
private:
    //uint16_t bitsPerValue_;
    uint64_t max_value_;
};

#endif