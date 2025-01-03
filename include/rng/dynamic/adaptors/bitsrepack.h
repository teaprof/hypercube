#ifndef __BITS_REPACK_H__
#define __BITS_REPACK_H__

#include <rng/dynamic/adaptors/adaptor.h>
#include <rng/dynamic/adaptors/circularbuffer.h>
#include <queue>


class BitsUnpack {
public:
    BitsUnpack(bool little_endian): little_endian_{little_endian} { }
    BitsUnpack(const BitsUnpack& other) : buffer_(other.buffer_), little_endian_{other.little_endian_} {}
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
    //CircularBuffer<bool> buffer_;
    bool little_endian_;
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
    BitsRepack(std::shared_ptr<RandomBitGenerator> rng, uint16_t dst_sample_size, bool src_little_endian, bool dst_little_endian) : RandomBitAdaptor(rng),
        bits_unpacker(src_little_endian), bits_packer(dst_sample_size, dst_little_endian) {}
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
#endif