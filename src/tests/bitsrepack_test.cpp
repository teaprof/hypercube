#include<rng/dynamic/adaptors/bitsrepack.h>
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
    uint16_t nbits() override {
        return nbits_;
    }
    std::shared_ptr<RandomBitGenerator> copy() override {
        return std::make_shared<FixedSequenceGenerator>(*this);
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

typedef testing::Types<BitsUnpackStd, BitsUnpackCircular, BitsUnpackFast> UnpackImplementations;
typedef testing::Types<std::pair<BitsUnpackStd, BitsPackStd>, std::pair<BitsUnpackCircular, BitsPackCircular>, std::pair<BitsUnpackFast, BitsPackFast>> RepackImplementations;

TYPED_TEST_SUITE(UnpackTest, UnpackImplementations);
TYPED_TEST_SUITE(RepackTest, RepackImplementations);


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

