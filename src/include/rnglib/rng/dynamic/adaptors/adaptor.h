#ifndef __ADAPTORS_H__
#define __ADAPTORS_H__

#include <rnglib/rng/dynamic/generators/RandomBitGenerator.h>
#include <memory>

class RandomBitAdaptor : public RandomBitGenerator {
    public:
    RandomBitAdaptor(std::shared_ptr<RandomBitGenerator> rng) : rng_(rng) {}
    RandomBitAdaptor(const RandomBitAdaptor& other) : rng_(other.rng_->copy()) {}
    virtual uint64_t operator()(RandomBitGenerator& rng) = 0;
    virtual uint64_t operator()() {return (*this)(*rng_);};
    virtual uint64_t max() {
        return (1<<(nbits())) - 1;
    }
    virtual uint16_t nbits() = 0;
private:
    std::shared_ptr<RandomBitGenerator> rng_;
};

#endif