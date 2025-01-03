#ifndef __BITS_REPACK_H__
#define __BITS_REPACK_H__

#include <rng/dynamic/adaptors/adaptor.h>
#include <rng/dynamic/adaptors/circularbuffer.h>
#include <queue>

template<class BoolQueue>
class BitsUnpackT {
public:
    BitsUnpackT(bool little_endian): little_endian_{little_endian} { }
    BitsUnpackT(const BitsUnpackT<BoolQueue>& other) : buffer_(other.buffer_), little_endian_{other.little_endian_} {}    

    bool pop_front(RandomBitGenerator &rng) {
        if (buffer_.empty()) {
            refill(rng);
        }
        bool res = buffer_.front();
        buffer_.pop_front();
        return res;
    };
private:        
    template<class ContainerT>
    void setCapacity(size_t maxCapacity)
    {
        //nothing to do
    }

    template<class ContainerT>
    void setCapacity(size_t maxCapacity)  requires requires(ContainerT obj) 
    {
        obj.setCapacity(maxCapacity);
    } 
    {
        buffer_.setCapacity(maxCapacity);
    }

    void refill(RandomBitGenerator& rng) {
        uint64_t val = rng();
        setCapacity<BoolQueue>(rng.nbits());
        if (little_endian_) {
            // start adding from lo byte
            for (size_t n = 0; n < rng.nbits(); n++) {
                buffer_.push_back(val % 2 != 0);
                val >>= 1;
            }
        } else {
            // start adding from hi bit
            uint64_t high_mask = static_cast<uint64_t>(1) << (rng.nbits() - 1);
            for (size_t n = 0; n < rng.nbits(); n++) {
                buffer_.push_back((val & high_mask) != 0);
                high_mask >>= 1;
            }
        }
    }
    BoolQueue buffer_;
    bool little_endian_;
};


template<class BoolQueue>
class BitsPackT {
public:
    BitsPackT(uint16_t sample_size_bits, bool little_endian): little_endian_{little_endian}, sample_size_bits_{sample_size_bits}{ }
    BitsPackT(const BitsPackT<BoolQueue>& other) : little_endian_{other.little_endian_}, sample_size_bits_{other.sample_size_bits_} {}
    uint64_t operator()(BitsUnpackT<BoolQueue>& bits_unpacked, RandomBitGenerator& rng) {
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
    uint64_t packLittleEndian(BitsUnpackT<BoolQueue>& bits_unpacked, RandomBitGenerator& rng) {
        uint64_t res = 0;
        uint64_t mask = 1;
        for(uint16_t n = 0; n < sample_size_bits_; n++) {
            if(bits_unpacked.pop_front(rng))
                res |= mask;
            mask<<=1;
        }
        return res;
    }
    uint64_t packBigEndian(BitsUnpackT<BoolQueue>& bits_unpacked, RandomBitGenerator& rng) {
        uint64_t res = 0;
        for(uint16_t n = 0; n < sample_size_bits_; n++) {
            res = (res<<1) + bits_unpacked.pop_front(rng);
        }
        return res;
    }
    bool little_endian_;
    uint16_t sample_size_bits_;
};

template<class BoolQueue>
class BitsRepackT : public RandomBitAdaptor {
public:
    BitsRepackT(std::shared_ptr<RandomBitGenerator> rng, uint16_t dst_sample_size, bool src_little_endian, bool dst_little_endian) : RandomBitAdaptor(rng),
        bits_unpacker(src_little_endian), bits_packer(dst_sample_size, dst_little_endian) {}
    BitsRepackT(const BitsRepackT<BoolQueue>& r) : RandomBitAdaptor(r), bits_unpacker(r.bits_unpacker), bits_packer(r.bits_packer) {}
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
        return std::make_shared<BitsRepackT<BoolQueue>>(*this);
    }
private:
    BitsUnpackT<BoolQueue> bits_unpacker;
    BitsPackT<BoolQueue> bits_packer;
};


using BitsRepackCircular = BitsRepackT<CircularQueue<bool>>;
using BitsRepackStd = BitsRepackT<std::deque<bool>>;

using BitsUnpack = BitsUnpackT<std::deque<bool>>;
using BitsPack = BitsPackT<std::deque<bool>>;
using BitsRepack = BitsRepackStd;
//using BitsRepack = BitsRepackCircular;
#endif