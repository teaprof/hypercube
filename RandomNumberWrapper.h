#ifndef RANDOM_NUMBER_WRAPPER_H
#define RANDOM_NUMBER_WRAPPER_H

#include <cassert>
#include <queue>
#include <random>
#include <vector>

class RandomBitGenerator {
public:
    virtual uint64_t operator()() = 0;
    virtual uint64_t max() = 0;
    virtual uint16_t nbits() = 0;
    virtual void discard() { (*this)(); }
    virtual void discardN(size_t n) {
        for (size_t i = 0; i < n; i++)
            discard();
    };
    virtual void jumpTo(uint64_t pos) { discardN(pos - counter); }
    uint64_t counter{0};
};

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
    virtual uint64_t operator()() {
        counter++;
        return rng();
    }
    virtual uint64_t max() { return rng.max(); }
    virtual uint16_t nbits() { return nbits_; }
private:
    uint16_t nbits_{0};
};

class RandomBitAdaptor : public RandomBitGenerator {
    public:
    RandomBitAdaptor(RandomBitGenerator& rng) : rng_(rng) {}
    virtual uint64_t operator()(RandomBitGenerator& rng) = 0;
    virtual uint64_t operator()() {return (*this)(rng_);};
    virtual uint64_t max() = 0;
    virtual uint16_t nbits() {
        uint16_t res = 0;
        uint64_t m = max();
        while(m) {
            res++;
            m>>=1;
        }
        return res;
    }
private:
    RandomBitGenerator& rng_;
};

class BitsUnpack {
public:
    BitsUnpack(uint16_t samlpe_size_bits, bool little_endian): little_endian_{little_endian}, sample_size_bits_{samlpe_size_bits}{ }
    bool pop_front(RandomBitGenerator &rng) {
        if (buffer_.empty()) {
            refill(rng);
        }
        bool res = buffer_.front();
        buffer_.pop();
        return res;
    };
private:
    void refill(RandomBitGenerator& rng) {
        uint64_t val = rng();
        if (little_endian_) {
            // start adding from lo byte
            for (size_t n = 0; n < rng.nbits(); n++) {
                buffer_.push(val % 2 != 0);
                val >>= 1;
            }
        } else {
            // start adding from hi bit
            uint64_t high_mask = 1 << (rng.nbits() - 1);
            for (size_t n = 0; n < rng.nbits(); n++) {
                buffer_.push(val % high_mask != 0);
                high_mask >>= 1;
            }
        }
    }
    std::queue<bool> buffer_;
    bool little_endian_;
    uint16_t sample_size_bits_;
};

class BitsPack {
public:
    BitsPack(uint16_t sample_size_bits, bool little_endian): little_endian_{little_endian}, sample_size_bits_{sample_size_bits}{ }
    uint64_t operator()(BitsUnpack& bits_unpacked, RandomBitGenerator& rng) {
        if(little_endian_) {
            return packLittleEndian(bits_unpacked, rng);
        }
        return packBigEndian(bits_unpacked, rng);
    }
    uint64_t max() {
        return (1<<sample_size_bits_) - 1;
    }
private:
    uint64_t packLittleEndian(BitsUnpack& bits_unpacked, RandomBitGenerator& rng) {
        uint64_t res = 0;
        uint64_t mask = 0;
        for(uint16_t n = 0; n < sample_size_bits_; n++) {
            if(bits_unpacked.pop_front(rng))
                res |= mask;
            mask<<=1;
        }
        return res;
    }
    uint64_t packBigEndian(BitsUnpack& bits_unpacked, RandomBitGenerator& rng) {
        uint64_t res = 0;
        for(uint16_t n = 0; n < sample_size_bits_; n++) {
            res = (res<<1) + bits_unpacked.pop_front(rng);
        }
        return res;
    }
    bool little_endian_;
    uint16_t sample_size_bits_;
};

class BitsRepack : public RandomBitAdaptor {
    BitsRepack(RandomBitGenerator& rng, uint16_t src_sample_size, uint16_t dst_sample_size, bool src_little_endian, bool dst_little_endian) : RandomBitAdaptor(rng),
        bits_unpacker(src_sample_size, src_little_endian), bits_packer(dst_sample_size, dst_little_endian) {}
    virtual uint64_t operator()(RandomBitGenerator& rng) override {
        return bits_packer(bits_unpacker, rng);
    }
    uint64_t max() override {
        return bits_packer.max();
    }
public:
    BitsUnpack bits_unpacker;
    BitsPack bits_packer;
};

class UniformIntDistribution : public RandomBitAdaptor {
    // Int should be power of two - if we want to investigate rng itself
    // Int should be obtained as std::uniform_distribution, but only single thread
    // Int should be calculated via float or double precision
public:
    UniformIntDistribution(RandomBitGenerator& rng, uint64_t max_value) : RandomBitAdaptor(rng), max_value_(max_value) {}
    virtual uint64_t operator()(RandomBitGenerator &rng) override {
        assert(max_value_ < (1 << (rng.nbits() - 1)));
        return rng() % (max_value_ + 1);
    }
    uint64_t max() override {
        return max_value_;
    }
private:
    uint16_t bitsPerValue_;
    uint64_t max_value_;
};

using MT19937Wrapper = RandomNumberWrapperStd<std::mt19937>;

#endif