/*
 * arithmetics.h
 *
 *  Created on: Mar 17, 2019
 *      Author: tea
 */

#ifndef a10RNG_ARITHMETICS_ARITHMETICS_H_
#define a10RNG_ARITHMETICS_ARITHMETICS_H_
#include<boost/integer.hpp>
#include<type_traits>
#include<concepts>
#include<stdexcept>

template<std::unsigned_integral UInt>
struct DoubleUInt
{
    DoubleUInt()
    {
    }
    DoubleUInt(UInt hi, UInt lo) : Hi(hi), Lo(lo)
    {
    }
    UInt Hi, Lo;
    void operator=(const DoubleUInt<UInt> rhs)
    {
        Hi = rhs.Hi;
        Lo = rhs.Lo;
    }
};

template<std::unsigned_integral UInt>
struct DoubleWidthUInt
{
    using fast = typename boost::uint_t<2*8*sizeof(UInt)>::fast;
    using exact = typename boost::uint_t<2*8*sizeof(UInt)>::exact;
    using least = typename boost::uint_t<2*8*sizeof(UInt)>::least;
};

template<>
struct DoubleWidthUInt<boost::uintmax_t>
{
    using exact = DoubleUInt<boost::uintmax_t>;
    using fast = exact;
    using least = exact;
};

template<typename UInt>
UInt addwithcarry(const UInt& a, const UInt& b, UInt& Carry)
{
    constexpr unsigned int intsize = sizeof(UInt)*8;
    constexpr unsigned int halfsize = intsize >> 1;
    UInt ahi = a >> halfsize;
    UInt alo = a - (ahi << halfsize);
    UInt bhi = b >> halfsize;
    UInt blo = b - (bhi << halfsize);
    UInt ResLo = alo + blo;
    UInt ResHi = ahi + bhi;
    Carry = ResHi >> halfsize;
    ResHi = ResHi - (Carry << halfsize);
    return ResLo + (ResHi <<halfsize);
}

template<std::unsigned_integral UInt, typename LongUInt = typename DoubleWidthUInt<UInt>::fast >
typename std::enable_if<std::is_same_v<LongUInt, DoubleUInt<UInt>>, LongUInt>::type
muladd(const UInt& a, const UInt& x, const UInt& b)
{
        constexpr unsigned int intsize = sizeof(UInt)*8;
	constexpr unsigned int halfsize = intsize >> 1;
        UInt ahi = a >> halfsize;
        UInt alo = a - (ahi<<halfsize);
        UInt xhi = x >> halfsize;
        UInt xlo = x - (xhi<<halfsize);
        UInt bhi = b >> halfsize;
        UInt blo = b - (bhi<<halfsize);
//  res = (ahi*2^halfsize + alo)*(xhi*2^halfsize + xlo) + bhi*2^halfsize + blo
//	res = ahi*xhi*2^intsize + (ahi*xlo + alo*xhi + bhi)*2^halfsize + alo*xlo + blo
	//find lower order digits
        UInt Lower = alo*xlo + blo;
        UInt Medium = ahi*xlo + alo*xhi + bhi;
        UInt High = ahi*xhi;

        UInt LowerHi = Lower >> halfsize;
        UInt LowerLo = Lower - (LowerHi<<halfsize);
        UInt MediumHi = Medium >> halfsize;
        UInt MediumLo = Medium - (MediumHi << halfsize);
        UInt SumLoHi = MediumLo + LowerHi;
        UInt Carry = SumLoHi >> halfsize;
	SumLoHi = SumLoHi - (Carry << halfsize);
        UInt SumLo = LowerLo + (SumLoHi<<halfsize);
        UInt SumHi = Carry + MediumHi + High;
        return DoubleUInt<UInt>(SumHi, SumLo);
}

template<std::unsigned_integral UInt, typename LongUInt = typename DoubleWidthUInt<UInt>::fast>
typename std::enable_if<std::is_integral<LongUInt>::value, LongUInt>::type
muladd(const UInt& a, const UInt& x, const UInt& b)
{
        LongUInt res = a;
        res *= x;
        res += b;
        return res;
}

template<std::unsigned_integral UInt, typename LongUInt = DoubleUInt<UInt> >
typename std::enable_if<std::is_same_v<DoubleUInt<UInt>, LongUInt>, LongUInt>::type
mul(const UInt& a, const UInt& x)
{
    constexpr unsigned int intsize = sizeof(UInt)*8;
    constexpr unsigned int halfsize = intsize >> 1;
    UInt ahi = a >> halfsize;
    UInt alo = a - (ahi<<halfsize);
    UInt xhi = x >> halfsize;
    UInt xlo = x - (xhi<<halfsize);
//  res = (ahi*2^halfsize + alo)*(xhi*2^halfsize + xlo)
//	res = ahi*xhi*2^intsize + (ahi*xlo + alo*xhi)*2^halfsize + alo*xlo
    //find lower order digits
    UInt Lower = alo*xlo;
    UInt Medium = ahi*xlo + alo*xhi;
    UInt High = ahi*xhi;

    UInt LowerHi = Lower >> halfsize;
    UInt LowerLo = Lower - (LowerHi<<halfsize);
    UInt MediumHi = Medium >> halfsize;
    UInt MediumLo = Medium - (MediumHi << halfsize);
    UInt SumLoHi = MediumLo + LowerHi;
    UInt Carry = SumLoHi >> halfsize;
    SumLoHi = SumLoHi - (Carry << halfsize);
    UInt SumLo = LowerLo + (SumLoHi<<halfsize);
    UInt SumHi = Carry + MediumHi + High;
    return DoubleUInt<UInt>(SumHi, SumLo);
}

template<std::unsigned_integral UInt>
void shiftleft(DoubleUInt<UInt>& a)
{
    constexpr unsigned int intsize = sizeof(UInt)*8;
    constexpr UInt HIBIT = static_cast<UInt>(1) << (intsize - 1);
    UInt sbit = HIBIT & a.Lo;
    a.Lo <<= 1;
    a.Hi <<= 1;
    if(sbit)
        a.Hi += 1;
}

template<std::unsigned_integral UInt, typename LongUInt>
LongUInt div(const LongUInt& dividend, UInt divisor, UInt &remainder)
/* This function can be optimized for case where it is known that
 * result of divident/divisor can be stored in Int, not DoubleInt.
 * In this case divident.Hi < divisor.
 */
{
    constexpr unsigned int intsize = sizeof(UInt)*8;
    LongUInt quotient;
    UInt i;
    remainder = 0;

    quotient = dividend;

    constexpr UInt HIBIT = static_cast<UInt>(1) << (intsize - 1);
    /* Use grade-school long division algorithm */
    for (i = 0; i < 2*intsize; i++)
    {
        UInt sbit = HIBIT & quotient.Hi;
        remainder <<= 1;
        if (sbit)
            remainder |= 1;
        shiftleft(quotient);
        if (remainder >= divisor)
        {
            remainder -= divisor;
            quotient.Lo |= 1;
        }
    }

    return quotient;
};

template<std::unsigned_integral UInt, typename LongInt = DoubleUInt<UInt>>
typename std::enable_if<std::is_same<LongInt, DoubleUInt<UInt>>::value, UInt>::type
mod(const DoubleUInt<UInt>& dividend, UInt divisor)
{
    UInt remainder;
    div(dividend, divisor, remainder);
    return remainder;
}

template<std::unsigned_integral UInt, typename DoubleUInt>
typename std::enable_if<std::is_integral<DoubleUInt>::value, UInt>::type
mod(DoubleUInt&& a, UInt m)
{
	return a % m;
}

template<std::unsigned_integral UInt, typename LongUInt>
LongUInt div(const LongUInt& dividend, UInt divisor)
{
    UInt remainder;
    DoubleUInt res = div(dividend, divisor, remainder);
    return res;
}

template<std::unsigned_integral UInt, typename DoubleUInt = typename DoubleWidthUInt<UInt>::fast>
DoubleUInt muldiv(const UInt&a , const UInt& b, const UInt& c)
///return a*b/c
{
    DoubleUInt ab = mul(a, b);
    DoubleUInt ab_div_c = div(ab, c);
    return ab_div_c;
}

template<std::unsigned_integral UInt, typename LongUInt = typename DoubleWidthUInt<UInt>::fast>
std::enable_if_t<std::is_same_v<LongUInt, DoubleUInt<UInt>>, UInt>
muldiv_short(const UInt&a , const UInt& b, const UInt& c)
///return a*b/c, call this function if you are sure that result can be stored int UInt (i.e. b<=c)
{
    using InnerType = typename DoubleWidthUInt<UInt>::fast;
    InnerType ab = mul(a, b);
    InnerType ab_div_c = div(ab, c);
    if(ab_div_c.Hi > 0)
        throw std::runtime_error("integer overflow");
    return ab_div_c.Lo;
}

template<std::unsigned_integral UInt>
std::enable_if_t<std::is_integral_v<DoubleWidthUInt<UInt>::fast>, UInt>
muldiv_short(const UInt&a , const UInt& b, const UInt& c)
///return a*b/c, call this function if you are sure that result can be stored int UInt (i.e. b<=c)
{
    using DoubleInt = typename DoubleWidthUInt<UInt>::fast;
    DoubleInt ab = static_cast<DoubleInt>(a)*b;
    DoubleInt ab_div_c = ab/c;
    if(ab_div_c > static_cast<DoubleInt>(boost::integer_traits<UInt>::max()))
        throw std::runtime_error("integer overflow");
    return static_cast<UInt>(ab_div_c);
}


#endif /* 10RNG_ARITHMETICS_ARITHMETICS_H_ */

