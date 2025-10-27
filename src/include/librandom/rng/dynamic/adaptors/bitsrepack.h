#ifndef __BITS_REPACK_H__
#define __BITS_REPACK_H__

#include <librandom/rng/dynamic/adaptors/adaptor.h>
#include <librandom/rng/dynamic/adaptors/circularbuffer.h>
#include <queue>
#include <bit>
#include <iostream>

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
    void discardN(uint64_t N, RandomBitGenerator &rng) {
        for(uint64_t n = 0; n < N; n++)
            pop_front(rng);
    }
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
    BitsPackT(uint_fast8_t sample_size_bits, bool little_endian): little_endian_{little_endian}, sample_size_bits_{sample_size_bits}{ }
    BitsPackT(const BitsPackT<BoolQueue>& other) : little_endian_{other.little_endian_}, sample_size_bits_{other.sample_size_bits_} {}
    uint64_t operator()(BitsUnpackT<BoolQueue>& bits_unpacked, RandomBitGenerator& rng) {
        if(little_endian_) {
            return packLittleEndian(bits_unpacked, rng);
        }
        return packBigEndian(bits_unpacked, rng);
    }
    void discardN(uint64_t N, BitsUnpackT<BoolQueue>& bits_unpacked, RandomBitGenerator& rng) {
        for(uint64_t n = 0; n < N; n++)
            this->operator()(bits_unpacked, rng);
    }
    uint64_t max() const {
        return (static_cast<uint64_t>(1)<<sample_size_bits_) - 1;
    }
    uint16_t nbits() const {
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
    uint_fast8_t sample_size_bits_;
};


template<class Unpacker, class Packer>
class BitsRepackT : public RandomBitAdaptor {
public:
    BitsRepackT(std::shared_ptr<RandomBitGenerator> rng, uint16_t dst_sample_size, bool src_little_endian, bool dst_little_endian) : RandomBitAdaptor(rng),
        bits_unpacker(src_little_endian), bits_packer(dst_sample_size, dst_little_endian) {}
    BitsRepackT(const BitsRepackT<Unpacker, Packer>& r) : RandomBitAdaptor(r), bits_unpacker(r.bits_unpacker), bits_packer(r.bits_packer) {}

    virtual uint64_t operator()(RandomBitGenerator& rng) override {
        return bits_packer(bits_unpacker, rng);
    }
    virtual uint64_t operator()() override {
        return (*this)(*rng());
        //return rng()->operator()();
    }
    void discardN(uint64_t N) override {
        bits_packer.discardN(N, bits_unpacker, *rng());
    }
    uint64_t max() const override {
        return bits_packer.max();
    }
    uint16_t nbits() const override {
        return bits_packer.nbits();
    }
    std::shared_ptr<RandomBitGenerator> copy() const override {
        return std::make_shared<BitsRepackT<Unpacker, Packer>>(*this);
    }
    std::vector<char> state() override {
        throw std::logic_error("Not implemented yet");
    }
    void setState(const std::vector<char>& state) override {
        throw std::logic_error("Not implemented yet");
    }
    size_t stateSize() override {
        throw std::logic_error("Not implemented yet");
    }

private:
    Unpacker bits_unpacker;
    Packer bits_packer;
};

template<class IntType>
void reverseBits(IntType& val, uint_fast8_t nbits) {
    IntType mask1 = static_cast<uint64_t>(1) << (nbits - 1);
    IntType mask2 = 1;
    //uint_fast8_t nbits_2 = nbits/2;
    while(mask1 > mask2) {
        bool v1 = (val & mask1);
        bool v2 = (val & mask2);
        if(v1 != v2) {
            if(v1) {
                val &= ~mask1;
                val |= mask2;
            } else {
                val |= mask1;
                val &= ~mask2;
            }
        }
        mask1 >>= 1;
        mask2 <<= 1;
    }
}

class BitsUnpackFast {
public:
    BitsUnpackFast(bool little_endian): order{little_endian?std::endian::little : std::endian::big} { }
    BitsUnpackFast(const BitsUnpackFast& other) : order{other.order}, buf_(other.buf_), buf_len_(other.buf_len_) {}    

    bool pop_front(RandomBitGenerator &rng) {
        // \todo: this function is used in tests only?
        if (empty()) {
            refill(rng);
        }
        bool res = front();
        pop();
        return res;
    }
    uint64_t pop_front_little_endian(RandomBitGenerator &rng, uint_fast8_t nbits) { 
        assert(nbits != 0);
        assert(nbits <= 64);
        uint64_t res = 0;
        uint_fast8_t remaining = nbits;
        uint_fast8_t mask_len = 0;
        uint_fast8_t ready = 0;
        while(remaining > 0) {
            if(buf_len_ == 0) {
                refill(rng);
                assert(buf_len_ > 0);
            }
            mask_len = std::min(buf_len_, remaining);            
            uint64_t mask = (mask_len == 64)? ~uint64_t(0) : (uint64_t(1) << mask_len) - 1;
            uint64_t term = ((buf_ & mask) << ready);
            res += term;

            remaining -= mask_len;
            ready += mask_len;
            buf_ >>= mask_len;
            buf_len_ -= mask_len;
        }
        return res;
    }
    void discardN(uint64_t N, RandomBitGenerator& rng) {
        if(buf_len_ == 0) {
            refill(rng);
        }
        if(N < buf_len_) {
            buf_ >>= N;
            buf_len_ -= N;
            return;
        }
        // drop existing buffer
        N -= buf_len_;        
        buf_len_ = 0;
        // drop entire buffers
        uint64_t drop_count = N / rng.nbits();
        rng.discardN(drop_count);
        N %= rng.nbits();
        // drop rest of the bits
        discardN(N, rng);
    }
private:        
    void refill(RandomBitGenerator& rng) {
        assert(empty());
        uint64_t val = rng();
        buf_ = val;
        if (order == std::endian::big) {
           reverseBits(buf_, rng.nbits());
        }
        buf_len_ = rng.nbits();
    }
    bool front() {
        return buf_ % 2;
    }
    void pop() {
        assert(buf_len_ > 0);
        buf_len_--;
        buf_>>=1;
    }
    bool empty() {
        return buf_len_ == 0;
    }
    std::endian order;
    uint64_t buf_{0};
    uint_fast8_t buf_len_{0};

};

class BitsPackFast {
public:
    BitsPackFast(uint16_t sample_size_bits, bool little_endian): order{little_endian? std::endian::little : std::endian::big}, sample_size_bits_{sample_size_bits}{ }
    BitsPackFast(const BitsPackFast& other) : order{other.order}, sample_size_bits_{other.sample_size_bits_} {}
    uint64_t operator()(BitsUnpackFast& bits_unpacked, RandomBitGenerator& rng) {
        assert(sample_size_bits_ != 0);
        uint64_t val = bits_unpacked.pop_front_little_endian(rng, sample_size_bits_);
        if(order == std::endian::big) {
            reverseBits(val, sample_size_bits_);
        }
        return val;
    }
    uint64_t max() const {
        return (static_cast<uint64_t>(1)<<sample_size_bits_) - 1;
    }
    uint16_t nbits() const {
        return sample_size_bits_;
    }
    void discardN(uint64_t N, BitsUnpackFast& bits_unpacked, RandomBitGenerator& rng) {
        /*for(uint64_t n = 0; n < N; n++)
            this->operator()(bits_unpacked, rng);*/
        bits_unpacked.discardN(N*sample_size_bits_, rng);
    }
private:
    std::endian order;
    uint16_t sample_size_bits_;
};


using BitsUnpackStd = BitsUnpackT<std::deque<bool>>;
using BitsUnpackCircular = BitsUnpackT<CircularQueue<bool>>;

using BitsPackStd = BitsPackT<std::deque<bool>>;
using BitsPackCircular = BitsPackT<CircularQueue<bool>>;

using BitsRepackCircular = BitsRepackT<BitsUnpackCircular, BitsPackCircular>;
using BitsRepackStd = BitsRepackT<BitsUnpackStd, BitsPackStd>;
using BitsRepackFast = BitsRepackT<BitsUnpackFast, BitsPackFast>;

using BitsUnpack = BitsUnpackStd;
using BitsPack = BitsPackStd;
using BitsRepack = BitsRepackStd;
//using BitsRepack = BitsRepackCircular;
#endif
