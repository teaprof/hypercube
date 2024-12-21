/*
 * rng.h
 *
 *  Created on: Mar 7, 2019
 *      Author: tea
 */

#ifndef a10RNG_RNG_H_
#define a10RNG_RNG_H_

#include "static/rngconcept.h"
#include "auxiliary/arithmetics.h"

#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace mctools
{

/*
 * Int - тип целого
 * a, c, m - параметры
 * state0 - начальное состояние
 * DoubleInt - целочисленный тип, используемый при внутренних преобразованиях
 *
 * satisfies RNGzerobasedConcept
 */
template<std::unsigned_integral UInt,
         UInt a, UInt c, UInt m, UInt state0,
         class LongUInt = typename DoubleWidthUInt<UInt>::fast>
class LCG
{
public:   
    using result_type = UInt;
	LCG() : state(state0), counter(0)
	{
	}
    //Following declarations make this class meet the uniform random bit generator requirements
    result_type operator()()
	{
        UInt res = state;
		next();
		return res;
	}
    static constexpr result_type min()
    {
        return minValue;
    }
    static constexpr result_type max()
    {
        return maxValue; /// \todo max should be in 0...m-1
    }
    result_type getState()
    {
        return state;
    }
protected:
    constexpr static UInt minValue = 0; //if c == 0 then minValue should be equal to 1
    constexpr static UInt maxValue = m - 1;
    UInt state;
    uintmax_t counter;

    //advance RNG to the new state by formula
    //state = (a*state + c) % m;
    void next()
    {
        LongUInt mm = muladd<UInt, LongUInt>(a, state, c);
        state = mod<UInt, LongUInt>(std::move(mm), m);
        counter++;
    }
};

constexpr uint32_t const2pow31 = powerOfTwo<31>::val; //2147483647

using tea_MINSTD = LCG<uint32_t, 48271, 0, const2pow31 - 1, 1, uint64_t>;
using tea_MINSTD2 = LCG<uint_fast32_t, 48271, 0, const2pow31 - 1, 1, uint64_t>;

//this generator's are local implementation of std::minstdrand0 and std::minstd_rand
using tea_minstd_rand0 = LCG<uint_fast32_t, 16807, 0, const2pow31 - 1, 1, uint_fast64_t>;
using tea_minstd_rand = LCG<uint_fast32_t, 48271, 0, const2pow31 - 1, 1, uint_fast64_t>;


static_assert(uint_random_generator0<tea_MINSTD>);
static_assert(uint_random_generator0<tea_MINSTD2>);
static_assert(uint_random_generator0<tea_minstd_rand0>);
static_assert(uint_random_generator0<tea_minstd_rand>);


} /* namespace mctools */

#endif /* 10RNG_RNG_H_ */
