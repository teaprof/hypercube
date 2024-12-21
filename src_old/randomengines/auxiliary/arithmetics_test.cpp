/*
 * arithmetics_test.cpp
 *
 *  Created on: Mar 17, 2019
 *      Author: tea
 */
#include"arithmetics.h"
#include<gtest/gtest.h>

void muladdTest(uint32_t a, uint32_t b, uint32_t c)
{
    DoubleUInt<uint32_t> Res = muladd<uint32_t, DoubleUInt<uint32_t>>(a, b, c);
    uint64_t res = uint64_t(a)*uint64_t(b) + uint64_t(c);
    uint32_t Hi = res >> 32;
    uint32_t Lo = res - (uint64_t(Hi)<<32);
    ASSERT_EQ(Res.Hi, Hi);
    ASSERT_EQ(Res.Lo, Lo);
}

void addWithCarryTest(uint32_t a, uint32_t b)
{
    uint32_t carry;
    uint32_t res = addwithcarry(a, b, carry);
    uint64_t res64 = uint64_t(a)+uint64_t(b);
    uint32_t Hi = res64 >> 32;
    uint32_t Lo = res64 - (uint64_t(Hi)<<32);
    ASSERT_EQ(Hi, carry);
    ASSERT_EQ(Lo, res);
}

TEST(muladd, Arithmetics)
{
    muladdTest(1, 1, 1);
    muladdTest(1e+5, 1e+5, 0);
    muladdTest(1e+9, 1e+9, 1e+9);
}
TEST(addwithcarry, Arithmetics)
{
    addWithCarryTest(1, 1);
    addWithCarryTest(1e+5, 1e+5);
    addWithCarryTest(1e+9, 1e+9);
}

TEST(shiftleft, Arithmetics)
{
    uint32_t a = 12345;
    uint64_t a64 = a;
    DoubleUInt<uint32_t> L(0, a);
    for(int k = 0; k < 100; k++)
    {
        shiftleft(L);
        a64 <<= 1;
        uint32_t Hi = a64 >> 32;
        uint32_t Lo = a64 - ((uint64_t)Hi << 32);
        ASSERT_EQ(L.Hi, Hi);
        ASSERT_EQ(L.Lo, Lo);
    }
}

void testmod(uint32_t hi, uint32_t lo, uint32_t modulus)
{
    uint64_t a64 = hi;
    a64 <<= 32;
    a64 += lo;
    DoubleUInt<uint32_t> L(hi, lo);
    uint32_t rem = a64 % modulus;
    uint32_t rem2 = mod(std::move(L), modulus);
    ASSERT_EQ(rem, rem2);
}

TEST(mod, Arithmetics)
{
    testmod(0, 1413123, 21);
    testmod(23141, 0, 21);
    testmod(0xFFFFFFFF, 0xFFFFFFFF, 21);
}

void testdiv(uint32_t hi, uint32_t lo, uint32_t divisor)
{
    uint64_t a64 = hi;
    a64 <<= 32;
    a64 += lo;
    DoubleUInt<uint32_t> L(hi, lo);
    uint64_t quotient = a64 / divisor;
    DoubleUInt<uint32_t> quotient2 = div(L, divisor);
    ASSERT_EQ(quotient2.Hi, quotient>>32);
    ASSERT_EQ(quotient2.Lo, quotient & 0xFFFFFFFF);
}

TEST(div, Arithmetics)
{
    testdiv(0, 1413123, 21);
    testdiv(23141, 0, 21);
    testdiv(23141, 23238, 21);
    testdiv(0xFFFFFFFF, 0xFFFFFFFF, 21);
}
