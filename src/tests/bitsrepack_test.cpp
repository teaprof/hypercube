#include<librandom/rng/dynamic/adaptors/bitsrepack.h>
#include<librandom/rng/dynamic/generators/RandomNumberWrapper.h>
#include<librandom/distributions/Distribution.h>
#include<vector>
#include<sstream>
#include<gtest/gtest.h>

class FixedSequenceGenerator : public RandomBitGenerator {
    public:
    FixedSequenceGenerator(uint16_t nbits, const std::vector<uint64_t>& buf) : buf_(buf), nbits_{nbits} 
    {
        for(auto it : buf_) {
            assert(it <= max());
        }
    }
    uint64_t operator()() override {
        assert(!buf_.empty());
        auto res = buf_.front();
        buf_.erase(buf_.begin());
        return res;
    }
    uint16_t nbits() const override {
        return nbits_;
    }
    std::shared_ptr<RandomBitGenerator> copy() const override {
        return std::make_shared<FixedSequenceGenerator>(*this);
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
    std::vector<uint64_t> buf_;
    uint16_t nbits_;
};

template <class T>
class UnpackTest : public testing::Test {
};


template <class T>
class RepackTest : public testing::Test {
};

template <class T>
class RepackDiscardTest : public testing::Test {
};

template <class T>
class IntDistributionDiscardTest : public testing::Test {
};


typedef testing::Types<BitsUnpackStd, BitsUnpackCircular, BitsUnpackFast> UnpackImplementations;
typedef testing::Types<std::pair<BitsUnpackStd, BitsPackStd>, std::pair<BitsUnpackCircular, BitsPackCircular>, std::pair<BitsUnpackFast, BitsPackFast>> RepackImplementations;

TYPED_TEST_SUITE(UnpackTest, UnpackImplementations);
TYPED_TEST_SUITE(RepackTest, RepackImplementations);
TYPED_TEST_SUITE(RepackDiscardTest, RepackImplementations);
TYPED_TEST_SUITE(IntDistributionDiscardTest, RepackImplementations);


TYPED_TEST(UnpackTest, BitsUnpackTestSimple)
{    
    FixedSequenceGenerator gen1(8, {0b00000001, 0b00000010});
    TypeParam unpackerBigEndian(false);
    std::stringstream str1;
    for(size_t n = 0; n < 16; n++)
        str1<<unpackerBigEndian.pop_front(gen1);
    ASSERT_EQ(str1.str(), "0000000100000010");

    FixedSequenceGenerator gen2(8, {0b00000001, 0b00000010});
    TypeParam unpackerLittleEndian(true);
    std::stringstream str2;
    for(size_t n = 0; n < 16; n++)
        str2<<unpackerLittleEndian.pop_front(gen2);
    ASSERT_EQ(str2.str(), "1000000001000000");
}



TYPED_TEST(RepackTest, BitsPackSimpleLittleEndian) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    FixedSequenceGenerator gen1(8, {0x00, 0x08, 0x10, 0x55});
    BitsUnpackType unpacker(true);
    BitsPackType packerLittleEndian(8, true);
    auto res1 = packerLittleEndian(unpacker, gen1);
    auto res2 = packerLittleEndian(unpacker, gen1);
    auto res3 = packerLittleEndian(unpacker, gen1);
    auto res4 = packerLittleEndian(unpacker, gen1);
    ASSERT_EQ(res1, 0x00);
    ASSERT_EQ(res2, 0x08);
    ASSERT_EQ(res3, 0x10);
    ASSERT_EQ(res4, 0x55);
}


TYPED_TEST(RepackTest, BitsPackSimpleBigEndian) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    FixedSequenceGenerator gen1(8, {0b00110011, 0b11000000});
    BitsUnpackType unpacker(true);
    BitsPackType packerBigEndian(8, false);
    auto res1 = packerBigEndian(unpacker, gen1);
    auto res2 = packerBigEndian(unpacker, gen1);
    ASSERT_EQ(res1, 0b11001100);
    ASSERT_EQ(res2, 0b00000011);
}


TYPED_TEST(RepackTest, BitsPackDoubleLittleEndian) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    FixedSequenceGenerator gen1(8, {0x00, 0x08, 0x10, 0x55});
    BitsUnpackType unpacker(true);
    BitsPackType packerLittleEndian(16, true);
    auto res1 = packerLittleEndian(unpacker, gen1);
    auto res2 = packerLittleEndian(unpacker, gen1);
    ASSERT_EQ(res1, 0x0800);
    ASSERT_EQ(res2, 0x5510);
}

TYPED_TEST(RepackTest, BitsPackDoubleBigEndian) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    FixedSequenceGenerator gen1(8, {0b10000011, 0b10100001});
    BitsUnpackType unpacker(true);
    BitsPackType packerBigEndian(16, false);
    auto res1 = packerBigEndian(unpacker, gen1);
    ASSERT_EQ(res1, 0b1100000110000101);
}

TYPED_TEST(RepackTest, BitsPackSameOrderLittleEndian) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    FixedSequenceGenerator gen1(16, {0xCAFE, 0xABCD});
    BitsUnpackType unpacker(true);
    BitsPackType packerLittleEndian(16, true);
    auto res1 = packerLittleEndian(unpacker, gen1);
    auto res2 = packerLittleEndian(unpacker, gen1);
    ASSERT_EQ(res1, 0xCAFE);
    ASSERT_EQ(res2, 0xABCD);
}

TYPED_TEST(RepackTest, BitsPackSameOrderBigEndian) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    FixedSequenceGenerator gen1(16, {0xCAFE, 0xABCD});
    BitsUnpackType unpacker(false);
    BitsPackType packerBigEndian(16, false);
    auto res1 = packerBigEndian(unpacker, gen1);
    auto res2 = packerBigEndian(unpacker, gen1);
    ASSERT_EQ(res1, 0xCAFE);
    ASSERT_EQ(res2, 0xABCD);
}

TYPED_TEST(RepackTest, BitsRepack6bitBEtoBE) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    FixedSequenceGenerator gen1(16, {0x1234, 0xABCD});
    BitsUnpackType unpacker(false);
    BitsPackType packerLittleEndian(6, false);
    auto res1 = packerLittleEndian(unpacker, gen1);
    auto res2 = packerLittleEndian(unpacker, gen1);
    auto res3 = packerLittleEndian(unpacker, gen1);
    auto res4 = packerLittleEndian(unpacker, gen1);
    auto res5 = packerLittleEndian(unpacker, gen1);
    ASSERT_EQ(res1, 4);
    ASSERT_EQ(res2, 35);
    ASSERT_EQ(res3, 18);
    ASSERT_EQ(res4, 43);
    ASSERT_EQ(res5, 51);
}


TYPED_TEST(RepackTest, BitsRepack6bitBEtoLE) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    FixedSequenceGenerator gen1(16, {0x1234, 0xABCD});
    BitsUnpackType unpacker(false);
    BitsPackType packerLittleEndian(6, true);
    auto res1 = packerLittleEndian(unpacker, gen1);
    auto res2 = packerLittleEndian(unpacker, gen1);
    auto res3 = packerLittleEndian(unpacker, gen1);
    auto res4 = packerLittleEndian(unpacker, gen1);
    auto res5 = packerLittleEndian(unpacker, gen1);
    ASSERT_EQ(res1, 8);
    ASSERT_EQ(res2, 49);
    ASSERT_EQ(res3, 18);
    ASSERT_EQ(res4, 53);
    ASSERT_EQ(res5, 51);
}

TYPED_TEST(RepackTest, BitsRepack6bitLEtoBE) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    FixedSequenceGenerator gen1(16, {0x1234, 0xABCD});
    BitsUnpackType unpacker(true);
    BitsPackType packerLittleEndian(6, false);
    auto res1 = packerLittleEndian(unpacker, gen1);
    auto res2 = packerLittleEndian(unpacker, gen1);
    auto res3 = packerLittleEndian(unpacker, gen1);
    auto res4 = packerLittleEndian(unpacker, gen1);
    auto res5 = packerLittleEndian(unpacker, gen1);
    ASSERT_EQ(res1, 11);
    ASSERT_EQ(res2, 4);
    ASSERT_EQ(res3, 34);
    ASSERT_EQ(res4, 51);
    ASSERT_EQ(res5, 53);
}


TYPED_TEST(RepackTest, BitsRepack6bitLEtoLE) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    FixedSequenceGenerator gen1(16, {0x1234, 0xABCD});
    BitsUnpackType unpacker(true);
    BitsPackType packerLittleEndian(6, true);
    auto res1 = packerLittleEndian(unpacker, gen1);
    auto res2 = packerLittleEndian(unpacker, gen1);
    auto res3 = packerLittleEndian(unpacker, gen1);
    auto res4 = packerLittleEndian(unpacker, gen1);
    auto res5 = packerLittleEndian(unpacker, gen1);
    ASSERT_EQ(res1, 52);
    ASSERT_EQ(res2, 8);
    ASSERT_EQ(res3, 17);
    ASSERT_EQ(res4, 51);
    ASSERT_EQ(res5, 43);
}


TYPED_TEST(RepackDiscardTest, BitsRepackLEtoLE) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    MT19937Wrapper gen1;
    BitsUnpackType unpacker1(true);
    BitsPackType packer1(6, true);
    std::vector<uint64_t> res1;
    const size_t N = 1000;
    for(size_t n = 0; n < N; n++) {
        //discard 
        for(size_t k = 0; k < 10; k++)
            packer1(unpacker1, gen1);
        //sample and store
        uint64_t v = packer1(unpacker1, gen1);
        res1.push_back(v);
        v = packer1(unpacker1, gen1);
        res1.push_back(v);
        //discard 
        for(size_t k = 0; k < 10; k++)
            packer1(unpacker1, gen1);
    }

    MT19937Wrapper gen2;
    BitsUnpackType unpacker2(true);
    BitsPackType packer2(6, true);
    std::vector<uint64_t> res2;
    for(size_t n = 0; n < N; n++) {
        packer2.discardN(10, unpacker2, gen2);
        uint64_t v = packer2(unpacker2, gen2);
        res2.push_back(v);
        v = packer2(unpacker2, gen2);
        res2.push_back(v);
        packer2.discardN(10, unpacker2, gen2);        
    }

    ASSERT_EQ(res1, res2);
}

TYPED_TEST(RepackDiscardTest, BitsRepackLEtoBE) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    MT19937Wrapper gen1;
    BitsUnpackType unpacker1(false);
    BitsPackType packer1(6, true);
    std::vector<uint64_t> res1;
    const size_t N = 1000;
    for(size_t n = 0; n < N; n++) {
        //discard 
        for(size_t k = 0; k < 10; k++)
            packer1(unpacker1, gen1);
        //sample and store
        uint64_t v = packer1(unpacker1, gen1);
        res1.push_back(v);
        v = packer1(unpacker1, gen1);
        res1.push_back(v);
        //discard 
        for(size_t k = 0; k < 10; k++)
            packer1(unpacker1, gen1);
    }

    MT19937Wrapper gen2;
    BitsUnpackType unpacker2(false);
    BitsPackType packer2(6, true);
    std::vector<uint64_t> res2;
    for(size_t n = 0; n < N; n++) {
        packer2.discardN(10, unpacker2, gen2);
        uint64_t v = packer2(unpacker2, gen2);
        res2.push_back(v);
        v = packer2(unpacker2, gen2);
        res2.push_back(v);
        packer2.discardN(10, unpacker2, gen2);        
    }

    ASSERT_EQ(res1, res2);
}

TYPED_TEST(RepackDiscardTest, BitsRepackBEtoLE) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    MT19937Wrapper gen1;
    BitsUnpackType unpacker1(true);
    BitsPackType packer1(6, false);
    std::vector<uint64_t> res1;
    const size_t N = 1000;
    for(size_t n = 0; n < N; n++) {
        //discard 
        for(size_t k = 0; k < 10; k++)
            packer1(unpacker1, gen1);
        //sample and store
        uint64_t v = packer1(unpacker1, gen1);
        res1.push_back(v);
        v = packer1(unpacker1, gen1);
        res1.push_back(v);
        //discard 
        for(size_t k = 0; k < 10; k++)
            packer1(unpacker1, gen1);
    }

    MT19937Wrapper gen2;
    BitsUnpackType unpacker2(true);
    BitsPackType packer2(6, false);
    std::vector<uint64_t> res2;
    for(size_t n = 0; n < N; n++) {
        packer2.discardN(10, unpacker2, gen2);
        uint64_t v = packer2(unpacker2, gen2);
        res2.push_back(v);
        v = packer2(unpacker2, gen2);
        res2.push_back(v);
        packer2.discardN(10, unpacker2, gen2);        
    }

    ASSERT_EQ(res1, res2);
}

TYPED_TEST(RepackDiscardTest, BitsRepackBEtoBE) {
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    MT19937Wrapper gen1;
    BitsUnpackType unpacker1(false);
    BitsPackType packer1(6, false);
    std::vector<uint64_t> res1;
    const size_t N = 1000;
    for(size_t n = 0; n < N; n++) {
        //discard 
        for(size_t k = 0; k < 10; k++)
            packer1(unpacker1, gen1);
        //sample and store
        uint64_t v = packer1(unpacker1, gen1);
        res1.push_back(v);
        v = packer1(unpacker1, gen1);
        res1.push_back(v);
        //discard 
        for(size_t k = 0; k < 10; k++)
            packer1(unpacker1, gen1);
    }

    MT19937Wrapper gen2;
    BitsUnpackType unpacker2(false);
    BitsPackType packer2(6, false);
    std::vector<uint64_t> res2;
    for(size_t n = 0; n < N; n++) {
        packer2.discardN(10, unpacker2, gen2);
        uint64_t v = packer2(unpacker2, gen2);
        res2.push_back(v);
        v = packer2(unpacker2, gen2);
        res2.push_back(v);
        packer2.discardN(10, unpacker2, gen2);        
    }

    ASSERT_EQ(res1, res2);
}

/// \todo: this is not unit test, it looks like integration one
TYPED_TEST(IntDistributionDiscardTest, LEtoLE) { 
    using BitsUnpackType = typename TypeParam::first_type;
    using BitsPackType = typename TypeParam::second_type;
    auto gen1 = std::make_shared<MT19937Wrapper>();
    UniformIntDistributionRough dist(16);
    BitsRepackT<BitsUnpackType, BitsPackType> repack1(gen1, 16, true, true);
    std::vector<uint64_t> res1;
    const size_t N = 1000;
    for(size_t n = 0; n < N; n++) {
        //sample and store
        uint64_t v = dist(repack1);
        res1.push_back(v);
        //discard 
        for(size_t k = 0; k < 10; k++)
            dist(repack1);
    }

    auto gen2 = std::make_shared<MT19937Wrapper>();
    BitsRepackT<BitsUnpackType, BitsPackType> repack2(gen2, 16, true, true);
    std::vector<uint64_t> res2;
    for(size_t n = 0; n < N; n++) {
        uint64_t v = dist(repack2);
        dist.discardN(10, repack2);
        res2.push_back(v);
    }

    ASSERT_EQ(res1, res2);
}
