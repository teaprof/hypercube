#ifndef RANDOM_NUMBER_WRAPPER_H
#define RANDOM_NUMBER_WRAPPER_H

#include <librandom/rng/dynamic/generators/RandomBitGenerator.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <random>
#include <vector>
#include <memory>
#include <algorithm>

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
        //std::cout<<"counter = "<<counter<<std::endl;
        return rng();
    }
    void discard() override {
        rng.discard(1);
        counter++;
    }
    void discardN(uint64_t n) override {
        rng.discard(n);
        counter += n;
    }
    uint64_t max() const override { return rng.max(); }
    uint16_t nbits() const override { return nbits_; }
    std::shared_ptr<RandomBitGenerator> copy() const override {
        return std::make_shared<RandomNumberWrapperStd<Rng>>(*this);
    }
    std::vector<char> state() override {
        std::stringstream str;
        str << counter << " " << rng;
        std::string s = std::move(str.str());
        if(s.length() > state_size)
            throw std::runtime_error("Generator returned state that is too large.");
        std::vector<char> res(state_size, 0);
        std::copy(s.begin(), s.end(), res.begin());
        return res;
    }
    void setState(const std::vector<char>& state) override {
        std::string str;
        str.assign(state_size, 0);
        std::copy(state.begin(), state.end(), str.begin());
        std::stringstream stream(std::move(str));
        stream >> counter >> rng;
    }
    size_t stateSize() override {
        return state_size;        
    }

private:
    static constexpr size_t state_size{10000};
    uint16_t nbits_{0};
};

using MT19937Wrapper = RandomNumberWrapperStd<std::mt19937>;

#endif