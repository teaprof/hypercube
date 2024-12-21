/*
 * standardrng.h
 *
 *  Created on: Apr 16, 2019
 *      Author: tea
 */

#ifndef STANDARDRNG_H_
#define STANDARDRNG_H_

#include<cstdint>
#include<type_traits> //for std::enable_if
//#include<a10engines/random_engine.h>
#include<a30hierarchy/rngbase.h>

namespace mctools
{

//generate doubles from [0, 1]
//not used, 2del
template<class RNG, int bytes>
class rngUInt
{
	using Int = typename RNG::IntType;
	constexpr static uint8_t IntSize = sizeof(int);
	RNG rng;
public:
	uint8_t getuint8_t()
	{
		return uint8_t (rng() & 0xFF);
	}
	uint16_t getuint16_t();
	uint32_t getuint32_t();
	uint64_t getuint64_t();
};






} /* namespace mctools */

#endif /* STANDARDRNG_H_ */
