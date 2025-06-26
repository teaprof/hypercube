#ifndef RANDOM_NUMBER_WRAPPER_H
#define RANDOM_NUMBER_WRAPPER_H

#include <rnglib/rng/dynamic/generators/RandomBitGenerator.h>

#include <cassert>
#include <random>
#include <vector>
#include <memory>

template <class Rng>
class RandomNumberWrapperStd : public RandomBitGenerator {
public:
    Rng rng;
    RandomNumberWrapperStd() {
        nbits_ = 0;
        auto m = max();
        while (m > 0) {
            nbits_++;
            m /= 2;
        }
    }
    RandomNumberWrapperStd(const RandomNumberWrapperStd<Rng>& other) : RandomBitGenerator(other), rng{other.rng}, nbits_{other.nbits_} {}
    uint64_t operator()() override {
        counter++;
        return rng();
    }
    uint64_t max() override { return rng.max(); }
    uint16_t nbits() override { return nbits_; }
    std::shared_ptr<RandomBitGenerator> copy() override {
        return std::make_shared<RandomNumberWrapperStd<Rng>>(*this);
    }
private:
    uint16_t nbits_{0};
};

using MT19937Wrapper = RandomNumberWrapperStd<std::mt19937>;

#endif