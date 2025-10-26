#ifndef _DYNAMIC_RANDOM_H_
#define _DYNAMIC_RANDOM_H_

#include <librandom/rng/static/lcg.h>

#include <memory>

class RandomBitGenerator {
public:
    virtual ~RandomBitGenerator() {}
    virtual uint64_t operator()() = 0;
    virtual uint16_t nbits() const = 0;
    virtual std::shared_ptr<RandomBitGenerator> copy() const = 0;

    virtual uint64_t max() const {
        return (static_cast<uint64_t>(1)<<nbits()) - 1;
    }
    virtual void discard() { (*this)(); }
    virtual void discardN(uint64_t n) {
        for (uint64_t i = 0; i < n; i++)
            discard();
    };
    virtual void jumpTo(uint64_t pos) { discardN(pos - counter); }
    virtual std::vector<char> state() = 0;
    virtual void setState(const std::vector<char>& state) = 0;
    virtual size_t stateSize() = 0;
    uint64_t counter{0};
};


#endif