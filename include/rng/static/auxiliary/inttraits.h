#ifndef INTTRAITS_H
#define INTTRAITS_H

#include <vector> //for size_t
#include <type_traits>
#include <inttypes.h>

namespace mctools {

template<size_t v1, size_t v2>
constexpr bool static_less()
{
    if constexpr (v1 < v2)
        return true;
    else
        return false;
}

template<size_t v1, size_t v2>
constexpr bool static_less_or_equal()
{
    if constexpr (v1 <= v2)
        return true;
    else
        return false;
}


/*! Bitmask<count>::val is a number of form 00..011..1 where ones are repeated count times.
 *  In other words, Bitmask<count>::val = 2^count - 1
 *  This can not be implemented simply as 2 << count - 1 because of overflow if count == sizeof(uintmax_t).
 */
template<size_t count>
struct bitmask
{
    static constexpr uintmax_t val = (bitmask<count-1>::val << 1) + 1;
};

template<>
struct bitmask<0>
{
    static constexpr uintmax_t val = 0;
};


/* nsignificantbits<targetValue>::nbits is a number of the most significant non-zero bit of targetValue.
 * nsignificantbits<targetValue>::mask is 2^nbits - 1 = 00..011..1 where the number of ones is equal to val.
 *
 * Properties:
 * 		1. 2^nbits > targetValue
 * 		2. mask = 2^nbits - 1
 * 		3. mask >= targetValue
 * 		4. if targetValue == 0, nbits and mask are zeros
 */
template<uintmax_t targetValue>
struct nsignificantbits
{
	static constexpr size_t nbits = nsignificantbits<(targetValue >> 1)>::nbits + 1;
    static constexpr uintmax_t mask   = bitmask<nbits>::val;
};

template<>
struct nsignificantbits<0>
{
	static constexpr size_t nbits = 0;
	static constexpr uintmax_t mask   = 0;
};


/* Integer division with rounding up.
 * divideRoundingTowards<dividend, divisor>::val = [cell(dividend/divisor)], where [x] denotes an integral part of x.
 * Properties:
 * 		1. val*divisor >= dividend
 * 		2. (val-1)*divisor < dividend
 */
template<uintmax_t dividend, uintmax_t divisor>
struct IntCell
{
	static constexpr uintmax_t val = dividend % divisor == 0 ? dividend/divisor : dividend/divisor + 1;
};


/*! \brief Declares value 2^bits and 2^bits-1
 *  \details
 *  This struct contains two useful constants: \p powerOfTwo = 2^bits and \p maxValue = 2^(bits-1).
 */
struct IntTraits
{
    /*! \brief return 2^bits
     *  \details maxValue<IntType, bits> returns value of type IntType like 2^bits.
     *  This function enabled only if bits < 8*sizeof(IntType) (i.e. IntType can store such value)
     */
    template<typename IntType, size_t bits>
    static constexpr
    typename std::enable_if<static_less<bits, 8*sizeof(IntType)>(), IntType>::type
    powerOfTwo()
    {
    	return IntType(1)<<bits;
    };

    /*! \brief returns 2^bits - 1
     *  \details Template function maxValue<IntType, bits> returns value of type IntType like
     *  2^bits - 1 = 00...011...1, where less significant \p bits bits are set to 1.
     *  This function enabled only if bits <= 8*sizeof(IntType).
     */
    template<typename IntType, size_t bits>
    static constexpr
    typename std::enable_if<static_less_or_equal<bits, 8*sizeof(IntType)>(), IntType>::type
    maxValue()
    {
    	if constexpr (bits == 0)
    		return 0;
    	else
    		return (maxValue<IntType, bits-1>() << 1) + 1;
    };

    template<uintmax_t targetValue>
    struct MaxMask
    {
        static constexpr uintmax_t val = nsignificantbits<targetValue>::nbits;
        static constexpr size_t maskBits = nsignificantbits<targetValue>::mask;
    };
};

template<uintmax_t src>
struct binaryDigits
{
    static constexpr uintmax_t src_shr = src >> 1;
    static constexpr size_t val = binaryDigits<src_shr>::val + 1;
};

template<>
struct binaryDigits<0>
{
    static constexpr size_t val = 0;
};

template<size_t n>
struct powerOfTwo
{
    static constexpr uintmax_t val = uintmax_t(1) << n;
};

/* \brief isPowerOfTwo<k>::val is true only if k is pow of two.
 * \details isPowerOfTwo<0> cause compilation error
 */
template<uintmax_t src>
struct isPowerOfTwo
{
    static_assert(src != 0, "src should be non-zero");
    static constexpr size_t digits = binaryDigits<src>::val;
    static constexpr bool val = powerOfTwo<digits - 1>::val == src;
};

//Some minimal tests to assure that arithmetic expressions above are correct
static_assert(bitmask<4>::val == 15);
static_assert(powerOfTwo<0>::val == 1);
static_assert(powerOfTwo<1>::val == 2);
static_assert(powerOfTwo<10>::val == 1024);
static_assert(binaryDigits<1>::val == 1);
static_assert(isPowerOfTwo<4>::val);

} /* namespace mctools */


#endif // INTTRAITS_H
