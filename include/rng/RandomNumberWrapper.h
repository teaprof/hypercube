#ifndef RANDOM_NUMBER_WRAPPER_H
#define RANDOM_NUMBER_WRAPPER_H

#include <rng/dynamic/random.h>

#include <cassert>
#include <queue>
#include <random>
#include <vector>
#include <memory>

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
    virtual std::shared_ptr<RandomBitGenerator> copy() = 0;
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

class BitsUnpack {
public:
    BitsUnpack(uint16_t samlpe_size_bits, bool little_endian): little_endian_{little_endian}, sample_size_bits_{samlpe_size_bits}{ }
    BitsUnpack(const BitsUnpack& other) : buffer_(other.buffer_), little_endian_{other.little_endian_}, sample_size_bits_{other.sample_size_bits_} {}
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
            uint64_t high_mask = static_cast<uint64_t>(1) << (rng.nbits() - 1);
            for (size_t n = 0; n < rng.nbits(); n++) {
                buffer_.push((val & high_mask) != 0);
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
    BitsPack(const BitsPack& other) : little_endian_{other.little_endian_}, sample_size_bits_{other.sample_size_bits_} {}
    uint64_t operator()(BitsUnpack& bits_unpacked, RandomBitGenerator& rng) {
        if(little_endian_) {
            return packLittleEndian(bits_unpacked, rng);
        }
        return packBigEndian(bits_unpacked, rng);
    }
    uint64_t max() {
        return (1<<sample_size_bits_) - 1;
    }
    uint16_t nbits() {
        return sample_size_bits_;
    }
private:
    uint64_t packLittleEndian(BitsUnpack& bits_unpacked, RandomBitGenerator& rng) {
        uint64_t res = 0;
        uint64_t mask = 1;
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
public:
    BitsRepack(std::shared_ptr<RandomBitGenerator> rng, uint16_t src_sample_size, uint16_t dst_sample_size, bool src_little_endian, bool dst_little_endian) : RandomBitAdaptor(rng),
        bits_unpacker(src_sample_size, src_little_endian), bits_packer(dst_sample_size, dst_little_endian) {}
    BitsRepack(const BitsRepack& r) : RandomBitAdaptor(r), bits_unpacker(r.bits_unpacker), bits_packer(r.bits_packer) {}
    virtual uint64_t operator()(RandomBitGenerator& rng) override {
        return bits_packer(bits_unpacker, rng);
    }
    uint64_t max() override {
        return bits_packer.max();
    }
    uint16_t nbits() override {
        return bits_packer.nbits();
    }
    std::shared_ptr<RandomBitGenerator> copy() override {
        return std::make_shared<BitsRepack>(*this);
    }
private:
    BitsUnpack bits_unpacker;
    BitsPack bits_packer;
};

using MT19937Wrapper = RandomNumberWrapperStd<std::mt19937>;

#endif