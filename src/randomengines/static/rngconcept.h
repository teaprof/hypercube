/*
 * rngconcept.h
 *
 *  Created on: Feb 11, 2020
 *      Author: tea
 */
#ifndef RNGCONCEPT_H
#define RNGCONCEPT_H

#include "auxiliary/inttraits.h"
#include <concepts>
#include <random>

/* This module defines some concepts related to random number generation.
 * The concepts defined here are:
 * - uint_random_generator
 * - uint_random_generator0
 * - uniform_random_bit_generator
 * - uniform_random_bit_generator_full_width
 * - Uab_generator
 * - U01_generator
 *
 * \todo: this uniform_random_bit_generator differs from std::uniform_random_bit_generator. Strictly saying,
 * std::uniform_random_bit_generator can generate non-uniformly distributed bits. Think, how to eliminate this
 * collision.
 *
 */

namespace mctools
{

//Below is a copy of uniform_random_bit_generator declaration from GNU C++ 10 standard library:
/*
template<typename _Gen>
  concept uniform_random_bit_generator
    = invocable<_Gen&> && unsigned_integral<invoke_result_t<_Gen&>>
    && requires
    {
      { _Gen::min() } -> same_as<invoke_result_t<_Gen&>>;
      { _Gen::max() } -> same_as<invoke_result_t<_Gen&>>;
      requires bool_constant<(_Gen::min() < _Gen::max())>::value;
    };
*/

//We add to this concept declaration some new requirements: the existance of the nested type result_type and
//the equvalence of this type to result type of operator(). These requirements are described in C++20 Standard, but we
//don't know why their are not added in std::uniform_random_bit_generator
template<typename Gen>
concept uint_random_generator
    = std::uniform_random_bit_generator<Gen>
      && requires(Gen& r, const Gen& cr)
{
    typename Gen::result_type;
    requires std::same_as<typename Gen::result_type, std::invoke_result_t<Gen&> >;
};


/* Uniform random bit generator with minimum return value equals to 0.
 * This is a special class of random bit generator that allows to write
 * 			rng()/rng.max()
 * instead of
 * 			(rng() - rng.min())/(rng.max() - rng.min())
 * because rng.min() = 0 for generators satisfying this concept.
 */
template<typename Gen>
concept uint_random_generator0 = std::uniform_random_bit_generator<Gen> && (Gen::min() == 0);


/*! Uniform random bit generator with max()+1 = 2^n and min() = 0. In this case the least significant
 * n bits are are sampled uniformly.
 *
 * Remark: std::uniform_random_bit_generator requires only that all values from min() to max() are
 * uniformly distributed, that is not equal that all bits are uniformly distributed.
 * (See C++20 26.6.3.3 "Uniform random bit generator requirements")
 */
template<typename Gen>
concept uniform_random_bit_generator =
		uint_random_generator0<Gen> && (nsignificantbits<Gen::max()>::mask  == Gen::max());


/* Uniform random bit generator with full width.
 * This is a uniform_random_bit_generator with additional requirement: Gen::max() should be equal to the max value
 * that can be stored in the Gen::result_type.
 * In this case all bits of the Gen() are uniformly distributed.
 *
 * Note: Special attention should be paid to the actual choice of the result_type, because if generator produce N-bits
 * integers, the result_type could be uint_fastN_t, that could be wider then N bits.
 */
template<typename Gen>
concept uniform_random_bit_generator_full_width =
		uniform_random_bit_generator<Gen> && (nsignificantbits<Gen::max()>::nbits  == sizeof(typename Gen::result_type)*8);


template<typename Gen>
concept Uab_generator =
        std::invocable<Gen&> && std::floating_point<std::invoke_result_t<Gen&> >
        && requires
        {
            { Gen::min() } -> std::same_as<std::invoke_result_t<Gen&>>;
            { Gen::max() } -> std::same_as<std::invoke_result_t<Gen&>>;
            requires std::bool_constant<(Gen::min() < Gen::max())>::value;
        };

template<typename Gen>
concept U01_generator =
        Uab_generator<Gen>
        && requires
        {
            requires std::bool_constant<Gen::min() == 0>::value;
            requires std::bool_constant<Gen::min() == 1>::value;
        };

} /* namespace mctools */

#endif // RNGCONCEPT_H
