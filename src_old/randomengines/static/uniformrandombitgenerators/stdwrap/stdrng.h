#ifndef STDRNG_H
#define STDRNG_H

#include"static/rngconcept.h"
#include<random>
#include<cassert>

namespace mctools
{

using std_minstd_rand = std::minstd_rand;
using std_minstd_rand0 = std::minstd_rand0;
using std_mt19937 = std::mt19937;
using std_mt19937_64 = std::mt19937_64;
using std_ranlux24 = std::ranlux24;
using std_ranlux24_base = std::ranlux24_base;
using std_ranlux48 = std::ranlux48;
using std_ranlux48_base = std::ranlux48_base;
using std_knuth_b = std::knuth_b;


//The goal of these tests is to show that all std random generators satisfy
//uint_random_generator concept
static_assert (uint_random_generator<std_minstd_rand>);
static_assert (uint_random_generator<std_minstd_rand0>);
static_assert (uint_random_generator<std_mt19937>);
static_assert (uint_random_generator<std_mt19937_64>);
static_assert (uint_random_generator<std_ranlux24>);
static_assert (uint_random_generator<std_ranlux24_base>);
static_assert (uint_random_generator<std_ranlux48>);
static_assert (uint_random_generator<std_ranlux48_base>);
static_assert (uint_random_generator<std_knuth_b>);


//The goal of these tests is to show that all std random generators satisfy
//uint_random_generator concept
//
//note: Some 32-bit RNGs (like std_mt19937) uses uint_fast32_t to represent result_type, that can be
//wider than 32 bits. So we do not require that std generators satisfy
//the uniform_random_bit_generator_full_width concept.
static_assert (uint_random_generator<std_minstd_rand>);
static_assert (uint_random_generator<std_minstd_rand0>);
static_assert (uniform_random_bit_generator<std_mt19937>);
static_assert (uniform_random_bit_generator<std_mt19937_64>);
static_assert (uniform_random_bit_generator<std_ranlux24>);
static_assert (uniform_random_bit_generator<std_ranlux24_base>);
static_assert (uniform_random_bit_generator<std_ranlux48>);
static_assert (uniform_random_bit_generator<std_ranlux48_base>);
static_assert (uint_random_generator<std_knuth_b>);

} /* namespace mctools */

#endif // STDRNG_H
