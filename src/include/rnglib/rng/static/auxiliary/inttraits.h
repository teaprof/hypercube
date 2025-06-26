#ifndef INTTRAITS_H
#define INTTRAITS_H

#include <vector> //for size_t
#include <type_traits>
#include <inttypes.h>

namespace mctools {

template<std::integral Int>
constexpr bool static_less(Int v1, Int v2)
{
    if (v1 < v2)
        return true;
    else
        return false;
}

template<std::integral Int>
constexpr bool static_less_or_equal(Int v1, Int v2)
{
    if (v1 <= v2)
        return true;
    else
        return false;
}


constexpr uintmax_t bitmask(size_t count) {
    if(count == 0) {
        return 0;
    } else {
        return 2*bitmask(count-1) + 1;
    }
}

// return a minimum number of bits sufficient to store targetValue
template<std::integral Int>
constexpr size_t nsignificantbits(Int val) {
    if(val == 0) {
        return 0;
    } else {
        return nsignificantbits(val>>1) + 1;
    }
}

template<std::integral Int>
constexpr uintmax_t minimalMask(Int val) {
    return (static_cast<uintmax_t>(1)<<nsignificantbits(val)) - 1;
}

template<std::integral Int>
constexpr uintmax_t IntCeil(Int dividend, Int divisor)
{
    return dividend % divisor == 0 ? dividend/divisor : dividend/divisor + 1;
};

template<std::integral Int>
constexpr uintmax_t powerOfTwo(Int p) {
    return static_cast<uintmax_t>(1) << p;
}


template<std::integral Int>
constexpr uintmax_t maxValue(Int nbits)
{
    static_assert(nbits >= 0);
    if (nbits == 0)
        return 0;
    else
        return 2*maxValue(nbits-1) + 1;
};

template<std::integral Int>
constexpr size_t binaryDigits(Int val) {
    if(val == 0) {
        return 0;
    } else {
        return binaryDigits(val>>1) + 1;
    }
}



template<std::integral Int>
constexpr bool isPowerOfTwo(Int val) {
    size_t digits = binaryDigits(val);
    return powerOfTwo(digits - 1) == val;
}


//Some minimal tests to ensure that arithmetic expressions above are correct
static_assert(bitmask(4) == 15);
static_assert(powerOfTwo(0) == 1);
static_assert(powerOfTwo(1) == 2);
static_assert(powerOfTwo(10) == 1024);
static_assert(powerOfTwo(32) == static_cast<uintmax_t>(1)<<32);
static_assert(binaryDigits(1) == 1);
static_assert(isPowerOfTwo(4));
static_assert(minimalMask(120) == 127);
static_assert(nsignificantbits(5)==3);
static_assert(nsignificantbits(15)==4);

} /* namespace mctools */


#endif // INTTRAITS_H
