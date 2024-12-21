/*
 * U01toInt.cpp
 *
 *  Created on: Apr 21, 2019
 *      Author: tea
 */

#include "static/rngconcept.h"
#include <cmath>
#include <set>
#include <gtest/gtest.h>
#include <algorithm>
#include "static/adapters/IntUniform.h"

using namespace mctools;

/*U01toInt::U01toInt(int Min, int Max)
{
	mMin = Min;
	mMax = Max;
}

U01toInt::~U01toInt() {
	// nothing to do
}

int U01toInt::operator()(double val)
{
    int res = mMin + floor(val*(mMax - mMin + 1));
    if(res > mMax) res = mMax; //это сработает только если val == 1
    return res;
}*/

struct DummyRng
{
    using result_type = unsigned char;
    std::vector<result_type> buf;
    size_t pos;
    DummyRng(std::initializer_list<result_type> list) : buf{list}, pos(0)
    {
    }
    result_type operator()()
    {
        if(pos == buf.size())
            throw std::logic_error("access past the last element");
        return buf[pos++];
    };
    static constexpr result_type min()
    {
        return 0;
    }
    static constexpr result_type max()
    {
        return std::numeric_limits<result_type>::max();
    }
    void reset()
    {
        pos = 0;
    }
};

static_assert(uint_random_generator0<DummyRng>);

TEST(a10random, UniformBits)
{
    DummyRng rng{0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    ASSERT_EQ(UniformBitsSequenceAdaptorFixedBits<DummyRng>::get8bits(rng), 0xAA);
    ASSERT_EQ(UniformBitsSequenceAdaptorFixedBits<DummyRng>::get16bits(rng), 0xBBCC);
    ASSERT_EQ(UniformBitsSequenceAdaptorFixedBits<DummyRng>::get32bits(rng), 0xDDEEFF11);
    ASSERT_EQ(UniformBitsSequenceAdaptorFixedBits<DummyRng>::get64bits(rng), 0x2233445566778899);
}

template<class result_type_, result_type_ min_, result_type_ max_>
struct DummyRngOrdered
{
    using result_type = result_type_;
    result_type cur;

    DummyRngOrdered(): cur(min_)
    {
    }
    result_type operator()()
    {
        result_type res = cur++;
        if(cur == max_)
            cur = min_;
        return res;
    };
    static constexpr result_type min()
    {
        return min_;
    }
    static constexpr result_type max()
    {
        return max_;
    }
    void reset()
    {
        cur = min_;
    }
};


template<class Int, Int srcmin, Int srcmax>
void testIntToRange(Int destmin, Int destmax)
{
    using RNG = DummyRngOrdered<Int, srcmin, srcmax>;
    std::vector<size_t> counts;
    counts.assign(destmax - destmin + 1, 0);
    RNG rng;
    for(size_t i = 1; i < srcmax - srcmin + 1; i++)
    {
        size_t idx = UniformIntSegment<RNG, size_t>::get(rng, destmin, destmax);
        std::cout<<idx<<" ";
        counts[idx - destmin]++;
    };
/*    std::cout<<std::endl;
    for(auto p : counts)
        std::cout<<p<<" ";
    std::cout<<std::endl;
    std::cout<<"aver = "<<double(srcmax - srcmin + 1)/(destmax - destmin + 1)<<std::endl;*/
    size_t maxelement = *std::max_element(counts.begin(), counts.end());
    size_t minelement = *std::min_element(counts.begin(), counts.end());
    ASSERT_LE(maxelement - minelement, 1);
}

TEST(a20adapters, IntToRange)
{
    using Int = unsigned int;
    /*testIntToRange<Int, 1, 18>(1, 1); //test if destmax = destmin
    testIntToRange<Int, 1, 18>(3, 10); //test if src range and dest range has not common divisor
    testIntToRange<Int, 4, 2000>(5, 9); //test if src range is must more larger than dest range
    testIntToRange<Int, 0, 999>(1, 10); //test if  src range is multiple dest ranges
    */
}

TEST(a20adapters, IntToDouble)
{
    using RNG = DummyRngOrdered<uint32_t, 0, boost::integer_traits<uint32_t>::max()>;
    RNG rng;
    double sumInc = 0;
    double sumExc = 0;
    for(size_t i = 0; i < 1e+5; i++)
    {
        sumInc += IntToDoubleExactSize<RNG, double>::getInclusive(rng);
        sumExc += IntToDoubleExactSize<RNG, double>::getExclusive(rng);
    }
    std::cout<<IntToDoubleExactSize<RNG, double>::bits<<std::endl;
    std::cout<<IntToDoubleExactSize<RNG, double>::bytes<<std::endl;
    using ttt = boost::int_t<56>::least;
    static constexpr ttt a = ttt(1) << 56;
    std::cout<<sizeof(ttt)<<std::endl;
}


TEST(rngconcepts, stdrng)
{
	using Gen = std::mt19937;
	std::cout<<nsignificantbits<Gen::max()>::mask<<std::endl;
	std::cout<<nsignificantbits<Gen::max()>::nbits<<std::endl;
	std::cout<<sizeof(typename Gen::result_type)*8<<std::endl;
	std::cout<<(nsignificantbits<Gen::max()>::nbits  == sizeof(typename Gen::result_type)*8)<<std::endl;
}
