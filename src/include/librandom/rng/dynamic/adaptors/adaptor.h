#ifndef __ADAPTORS_H__
#define __ADAPTORS_H__

#include <librandom/rng/dynamic/generators/RandomBitGenerator.h>
#include <memory>

class RandomBitAdaptor : public RandomBitGenerator {
    public:
    RandomBitAdaptor(std::shared_ptr<RandomBitGenerator> rng) : rng_(rng) {}
    RandomBitAdaptor(const RandomBitAdaptor& other) : rng_(other.rng_->copy()) {}

    virtual uint64_t operator()(RandomBitGenerator& rng) = 0;
    virtual uint64_t operator()() override {
        return (*this)(*rng_);
    };

    virtual uint64_t max() const override {
        return (static_cast<uint64_t>(1)<<(nbits())) - 1;        
    }

    virtual uint16_t nbits() const override = 0;

    std::shared_ptr<RandomBitGenerator> rng() { 
        return rng_; 
    }
    std::shared_ptr<const RandomBitGenerator> rng() const { 
        return rng_; 
    }

    void discardN(uint64_t count)  override {
        for(uint64_t n = 0; n < count; n++)
            this->operator()();
    }
private:
    std::shared_ptr<RandomBitGenerator> rng_;
};

#endif