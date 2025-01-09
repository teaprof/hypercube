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
class RepackTest : public testing::Test {
};

typedef testing::Types<std::deque<bool>, CircularQueue<bool>> Implementations;

TYPED_TEST_SUITE(RepackTest, Implementations);


TYPED_TEST(RepackTest, BitsUnpackTestSimple)
{    
    FixedSequenceGenerator gen1(8, {0b00000001, 0b00000010});
    BitsUnpackT<TypeParam> unpackerBigEndian(false);
    std::stringstream str1;
    for(size_t n = 0; n < 16; n++)
        str1<<unpackerBigEndian.pop_front(gen1);
    ASSERT_EQ(str1.str(), "0000000100000010");

    FixedSequenceGenerator gen2(8, {0b00000001, 0b00000010});
    BitsUnpackT<TypeParam> unpackerLittleEndian(true);
    std::stringstream str2;
    for(size_t n = 0; n < 16; n++)
        str2<<unpackerLittleEndian.pop_front(gen2);
    ASSERT_EQ(str2.str(), "1000000001000000");
}



TYPED_TEST(RepackTest, BitsPackSimpleLittleEndian) {
    FixedSequenceGenerator gen1(8, {0x00, 0x08, 0x10, 0x55});
    BitsUnpackT<TypeParam> unpacker(true);
    BitsPackT<TypeParam> packerLittleEndian(8, true);
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
    FixedSequenceGenerator gen1(8, {0b00110011, 0b11000000});
    BitsUnpackT<TypeParam> unpacker(true);
    BitsPackT<TypeParam> packerBigEndian(8, false);
    auto res1 = packerBigEndian(unpacker, gen1);
    auto res2 = packerBigEndian(unpacker, gen1);
    ASSERT_EQ(res1, 0b11001100);
    ASSERT_EQ(res2, 0b00000011);
}


TYPED_TEST(RepackTest, BitsPackDoubleLittleEndian) {
    FixedSequenceGenerator gen1(8, {0x00, 0x08, 0x10, 0x55});
    BitsUnpackT<TypeParam> unpacker(true);
    BitsPackT<TypeParam> packerLittleEndian(16, true);
    auto res1 = packerLittleEndian(unpacker, gen1);
    auto res2 = packerLittleEndian(unpacker, gen1);
    ASSERT_EQ(res1, 0x0800);
    ASSERT_EQ(res2, 0x5510);
}

TYPED_TEST(RepackTest, BitsPackDoubleBigEndian) {
    FixedSequenceGenerator gen1(8, {0b10000011, 0b10100001});
    BitsUnpackT<TypeParam> unpacker(true);
    BitsPackT<TypeParam> packerBigEndian(16, false);
    auto res1 = packerBigEndian(unpacker, gen1);
    ASSERT_EQ(res1, 0b1100000110000101);
}

TYPED_TEST(RepackTest, BitsPackSameOrderLittleEndian) {
    FixedSequenceGenerator gen1(16, {0xCAFE, 0xABCD});
    BitsUnpackT<TypeParam> unpacker(true);
    BitsPackT<TypeParam> packerLittleEndian(16, true);
    auto res1 = packerLittleEndian(unpacker, gen1);
    auto res2 = packerLittleEndian(unpacker, gen1);
    ASSERT_EQ(res1, 0xCAFE);
    ASSERT_EQ(res2, 0xABCD);
}

TYPED_TEST(RepackTest, BitsPackSameOrderBigEndian) {
    FixedSequenceGenerator gen1(16, {0xCAFE, 0xABCD});
    BitsUnpackT<TypeParam> unpacker(false);
    BitsPackT<TypeParam> packerBigEndian(16, false);
    auto res1 = packerBigEndian(unpacker, gen1);
    auto res2 = packerBigEndian(unpacker, gen1);
    ASSERT_EQ(res1, 0xCAFE);
    ASSERT_EQ(res2, 0xABCD);
}

TYPED_TEST(RepackTest, BitsRepack6bitBEtoBE) {
    FixedSequenceGenerator gen1(16, {0x1234, 0xABCD});
    BitsUnpackT<TypeParam> unpacker(false);
    BitsPackT<TypeParam> packerLittleEndian(6, false);
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
    FixedSequenceGenerator gen1(16, {0x1234, 0xABCD});
    BitsUnpackT<TypeParam> unpacker(false);
    BitsPackT<TypeParam> packerLittleEndian(6, true);
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
    FixedSequenceGenerator gen1(16, {0x1234, 0xABCD});
    BitsUnpackT<TypeParam> unpacker(true);
    BitsPackT<TypeParam> packerLittleEndian(6, false);
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
    FixedSequenceGenerator gen1(16, {0x1234, 0xABCD});
    BitsUnpackT<TypeParam> unpacker(true);
    BitsPackT<TypeParam> packerLittleEndian(6, true);
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

