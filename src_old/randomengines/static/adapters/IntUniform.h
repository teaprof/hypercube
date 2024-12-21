/*
 * U01toInt.h
 *
 *  Created on: Apr 21, 2019
 *      Author: tea
 */

#ifndef A20UNIFORM_U01TOINT_H_
#define A20UNIFORM_U01TOINT_H_

#include"auxiliary/inttraits.h"
#include"auxiliary/arithmetics.h"
#include"static/rngconcept.h"
#include<memory>
#include<cmath>
#include<limits>
#include<boost/integer.hpp>
#include<cassert>

namespace mctools {

/* Use cases:
 * uniform_random_bit_generator -> [0, 2^k-1] rejecting
 * uniform_random_bit_generator -> [0, 2^k-1]  linear (how to choose value of k?)
 * [0, 2^k-1] -> uint8_t, uint16_t, uint32_t, uint64_t
 * uniform_random_bit_generator -> segment[a, b] linear
 * uniform_random_bit_generator -> U[0, 1) as float using one RNG output
 * uniform_random_bit_generator -> U[0, 1] as float using one RNG output
 *
 * uniform_random_bit_generator -> [0, 2^k-1] -> uint_t<size> -> U[0, 1) as float<size>
 *                              -> U[0, 1) as float or as double
 *
 */

/// @todo
/// 1. +RNGtoZeroBased
/// 2. +RNGtoFixedWidthRejecting - max possible width, some outputs are rejected
/// 2. ZeroBasedToFullWidth_linear - max possible width, probalities of outputs are not exactly equal
/// 2. FullWidthToFixedWidth of 8, 16, 32, 64 bits
/// 3. RNGtoIntSegment [a, b]
/// 4. RNGtoUabSegment [a, b]
/// 5. RNGtoUabSemi [a, b)
///


template<std::uniform_random_bit_generator Gen>
class RNGtoZeroBased
{
    public:
        using result_t = std::invoke_result_t<Gen&>;
        static constexpr result_t min()
        {
            return 0;
        };
        static constexpr result_t max()
        {
            return Gen::max() - Gen::min();
        };
        result_t operator()(Gen& obj)
        {
            return obj() - Gen::min();
        };
};

/*!
 * \brief Converts output of some RNG to unsigned int type with a specified length, merging some RNG outputs if neccessary
 * \details Function RNG::operator()(RNG& rng) has the behavior as following
 * 1. Determine which bits of RNG reports distributed uniformly, mask these bits with uniform_bits_mask.
 * The number of uniform bits k is max{k :2^k - 1 <= RNG::max() - RNG::min()}.
 * 2. All outputs of an RNG greater than RNG::min() + 2^k - 1 are ignored.
 * 3. If the required integer size is less than sizeof(RNG::IntType) it uses exactly one outcome of
 * the RNG truncating most significant bits to form the integer of a specified size.
 * 4. If the required size is greater than sizeof(RNG::IntType), calls RNG() so many times as needed to
 * obtain integer with minimal size not less than sizeof(RNG::IntType). Then truncate the most significant bits
 * to form the integer of a specified size.
 *
 * Uses little-endian notation, i.e. if RNG generates sequence starting from 0xAA, 0xBB
 * then get16bits will return 0xAABB.
 */
template<std::uniform_random_bit_generator RNG, uint8_t required_bits>
class RNGToFixedWidthRejecting
{
public:
    static_assert (required_bits > 0, "n should be greater than zero");

    using result_type = typename boost::int_t<required_bits>::least;
    using RNG_result_type = std::invoke_result_t<RNG>;

    static constexpr result_type requiredBitsMask = bitmask<required_bits>::val;

    ///Analyzing the RNG properties:
    ///1. number of bits that are distributed uniformly
    static constexpr uint_fast8_t RNG_uniform_bits_count = IntTraits::MaxMask<RNG::max() - RNG::min()>::maskBits;

    ///2. mask consisting of uniform_bits_count bits
    static constexpr RNG_result_type uniform_bits_mask = IntTraits::MaxMask<RNG::max() - RNG::min()>::val;

    ///3. calculate required number of RNG outputs to merge into value of width required_bits bits
    static constexpr uint_fast8_t requiredOutputs = IntCell<required_bits, RNG_uniform_bits_count>::val;

    static RNG_result_type getUniformBits_rejecting(RNG& r)
    {
        RNG_result_type val;
        do {
            val = r() - RNG::min();
        } while(val > uniform_bits_mask);
        return val;
    }

    static result_type get(RNG &r)
    {        
        result_type res = 0;
        if constexpr (requiredOutputs == 1)
        {
            res = getUniformBits_rejecting(r);
        } else {
            for(size_t i = 0; i < requiredOutputs; i++)
            {
                res <<= RNG_uniform_bits_count;
                RNG_result_type val = getUniformBits_rejecting(r);
                //generate random number from [0...uniform_bits_mask]
                //because in this range bits are distributed uniformly
                res  += val;
            }
        };
        return res & requiredBitsMask;
    }

    result_type operator()(RNG &r)
    {
        return get(r);
    }

    static constexpr result_type min()
    {
        return 0;
    }
    static constexpr result_type max()
    {
        return requiredBitsMask;
    }
};

template<std::uniform_random_bit_generator RNG>
class RNGToFullWidthRejecting
{
public:
    ///Analyzing the RNG properties:
    ///1. number of bits that are distributed uniformly
    static constexpr uint_fast8_t RNG_uniform_bits_count = IntTraits::MaxMask<RNG::max() - RNG::min()>::maskBits;

};

template<class RNG, uint8_t numbersPerSample, uint8_t required_bits = 8*numbersPerSample*sizeof(RNG::IntType)>
class UniformBitsSequenceAdaptor2
{
public:
    using rngIntType = typename RNG::IntType;
    using IntType = boost::int_t<required_bits>;

    static IntType get(RNG& r)
    {
    }
};

template<class RNG>
class UniformBitsSequenceAdaptorFixedBits
{
public:
    template<class uInt>
    static uInt get(RNG& r)
    {
        return RNGToFixedWidthRejecting<RNG, sizeof(uInt)>::get(r);
    }
    static uint8_t get8bits(RNG& r)
    {
        return RNGToFixedWidthRejecting<RNG, 8>::get(r);
    }
    static uint16_t get16bits(RNG& r)
    {
        return RNGToFixedWidthRejecting<RNG, 16>::get(r);
    }
    static uint32_t get32bits(RNG& r)
    {
        return RNGToFixedWidthRejecting<RNG, 32>::get(r);
    }
    static uint64_t get64bits(RNG& r)
    {
        return RNGToFixedWidthRejecting<RNG, 64>::get(r);
    }
};

template<class RNG, class Double, uint_fast8_t bits>
class IntToDouble
{
public:
    static Double getInclusive(RNG& rng)
    {
        using DenominatorType = typename boost::uint_t<bits>::least;
        constexpr DenominatorType denom = IntTraits::maxValue<DenominatorType, bits>(); //2^bits - 1
        auto randombits = RNGToFixedWidthRejecting<RNG, bits>::get(rng);
        double v = static_cast<double>(randombits);
        return v/denom;
    }
    static Double getExclusive(RNG& rng)
    {
        using DenominatorType = typename boost::int_t<bits+1>::least;
        constexpr DenominatorType denom = IntTraits::powerOfTwo<DenominatorType, bits>(); //2^bits
        double v = static_cast<double>(RNGToFixedWidthRejecting<RNG, bits>::get(rng));
        return v/denom;
    }
};

template<class RNG, class Double>
class IntToDoubleExactSize
{
public:
    static Double getInclusive(RNG& rng)
    {
        return IntToDouble<RNG, Double, 8*bytes>::getInclusive(rng);
    }
    static Double getExclusive(RNG& rng)
    {
        return IntToDouble<RNG, Double, 8*bytes>::getExclusive(rng);
    }
    static constexpr size_t bits = std::numeric_limits<Double>::digits;
    static constexpr size_t bytes = std::ceil(static_cast<double>(bits)/8);
private:
};

/*! \brief Converts RNG outcome to integer in segment [Left, Right]
 */
template<class RNG, class Int>
class UniformIntSegment
{
public:
    static Int get(RNG& rng, Int Left, Int Right)
    {
        return Left + get(rng, Right - Left);
    }
    ///generates Int value in [0, Max]
    static Int get(RNG& rng, Int Max)
    {
        if(Max > rng.max() - rng.min())
            throw std::domain_error("Max should be less than RNG range");

        //generate random value in [0, RNG::max() - RNG::min()];
        typename RNG::result_type val = rng() - rng.min();

        //Monotonic map this value to [0, Max] with the following properties
        //  * maximum value of res equals to Max+1 and occures only if val = RNG::max - RNG::min.
        //  * the values are equally distributed i.e. if val runs [0, RNG::max()-RNG::min()]
        //  the numbers of hits to each possible value in [Left, Right] differs not greater than 1 (except last value)
        //any pairwise differ
        //Int res = val*(Max + 1)/(RNG::max() - RNG::min()); /// @todo static_cast to some wide integer
        auto temp = mul<Int, typename DoubleWidthUInt<Int>::fast>(val, Max + 1);
        typename DoubleWidthUInt<Int>::fast resLong = div(temp, rng.max() - rng.min()); /// @todo Max+1 could become zero on overflow
        assert(resLong.Hi == 0);
        Int res = resLong.Lo;
        //correct last value
        if(res > Max)
            res = Max;
        return res;
    }
};


} /* namespace mctools */

#endif /* A20UNIFORM_U01TOINT_H_ */
