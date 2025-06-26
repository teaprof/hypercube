#ifndef _DYNAMIC_RANDOM_H_
#define _DYNAMIC_RANDOM_H_

#include <rng/static/lcg.h>

#include <memory>

class RandomBitGenerator {
public:
    virtual ~RandomBitGenerator() {}
    virtual uint64_t operator()() = 0;
    virtual uint16_t nbits() = 0;
    virtual std::shared_ptr<RandomBitGenerator> copy() = 0;

    virtual uint64_t max() {
        return (static_cast<uint64_t>(1)<<nbits()) - 1;
    }
    virtual void discard() { (*this)(); }
    virtual void discardN(size_t n) {
        for (size_t i = 0; i < n; i++)
            discard();
    };
    virtual void jumpTo(uint64_t pos) { discardN(pos - counter); }
    uint64_t counter{0};
};


#endif