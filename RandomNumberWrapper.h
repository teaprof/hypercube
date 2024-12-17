#ifndef RANDOM_NUMBER_WRAPPER_H
#define RANDOM_NUMBER_WRAPPER_H

#include<random>
#include<vector>
#include<cassert>
#include<queue>

class RandomBitGenerator {
public:
    virtual uint64_t operator()() = 0;
    virtual uint64_t max() = 0;
    virtual uint16_t nbits() = 0;
    virtual void discard() {
        (*this)();
    }
    virtual void discardN(size_t n) {
        for(size_t i = 0; i < n; i++)
            discard();
    };
    virtual void jumpTo(uint64_t pos) {
        discardN(pos - counter);
    }
    uint64_t counter{0};
};

template<class Rng> 
class RandomNumberWrapperStd : public RandomBitGenerator {
public:
    Rng rng;
    RandomNumberWrapperStd() {
        nbits_ = 0;
        auto m = max();
        while(m > 0) {
            nbits_++;
            m /= 2;
        }
    }
    virtual uint64_t operator()() {
        counter++;
        return rng();
    }
    virtual uint64_t max() {
        return rng.max();
    }
    virtual uint16_t nbits () {
        return nbits_;
    }
private:
    uint16_t nbits_{0};
};

class NBitsGenerator {
    public:
    NBitsGenerator(RandomBitGenerator& rng, uint16_t nbits, bool srcLittleEndian, bool dstLittleEndian) : rng_(rng), nbits_(nbits), srcLittleEndian_(srcLittleEndian), dstLittleEndian_(dstLittleEndian) {}
    uint64_t operator()() {
        uint64_t res = 0;
        if(dstLittleEndian_) {
            //little endian => big first
            for(uint16_t n = 0; n < nbits_; n++) {
                res = (res<<2) + pop_front();
            }

        } else {
            //big endian => little first
            uint64_t mask = 1;
            for(uint16_t n = 0; n < nbits_; n++) {
                if(pop_front())
                    res |= mask;
                mask <<= 1;
            }
        }
        counter++;
        return res;
    }
    virtual void discard() {
        (*this)();
    }
    virtual void discardN(size_t n) {
        for(size_t i = 0; i < n; i++)
            discard();
    };
    virtual uint16_t nbits() {
        return nbits_;
    }
protected:
    bool pop_front() {
        if(buffer.empty()) {
            size_t val = rng_();
            if(srcLittleEndian_) {
                //start adding from hi bit
                uint64_t high_mask = 1 << (rng_.nbits() - 1);
                for(size_t n = 0; n < rng_.nbits(); n++) {
                    buffer.push(val % high_mask != 0);
                    high_mask >>= 1;                    
                }
            } else {
                //start adding from lo bit
                for(size_t n = 0; n < rng_.nbits(); n++) {
                    buffer.push(val % 2 != 0);
                    val >>= 1;
                }
            }
        }
        bool res =  buffer.front();
        buffer.pop();
        return res;
    };
    std::queue<bool> buffer;
    uint16_t nbits_;
    RandomBitGenerator& rng_;
    bool srcLittleEndian_;
    bool dstLittleEndian_;
    uint64_t counter{0};
};


class IntGenerator {
    //Int should be power of two - if we want to investigate rng itself
    //Int should be obtained as std::uniform_distribution, but only single thread
    //Int should be calculated via float or double precision
    public:       
        IntGenerator(NBitsGenerator& gen, uint16_t bitsPerValue) : gen_(gen), bitsPerValue_(bitsPerValue) {};
        virtual size_t operator()(size_t maxvalue) {
            assert(maxvalue < (1<<(gen_.nbits() - 1)));
            return gen_() % maxvalue;
        }
    private:
        uint16_t bitsPerValue_;   
        NBitsGenerator& gen_;
};


using MT19937Wrapper = RandomNumberWrapperStd<std::mt19937>;

#endif