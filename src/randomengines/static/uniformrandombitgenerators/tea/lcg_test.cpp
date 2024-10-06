/*
 * rng.cpp
 *
 *  Created on: Apr 12, 2019
 *      Author: tea
 */
#include "static/uniformrandombitgenerators/tea/lcg.h"

#include<gtest/gtest.h>
#include<chrono>


using namespace mctools;

TEST(next, LCG)
{
    tea_MINSTD rng1;
    tea_MINSTD2 rng2;
	for(size_t n = 0; n < 1000; n++)
	{
		rng1();
		rng2();
	}
	ASSERT_EQ(rng1.getState(), rng2.getState());
}

template<class rngtype>
void SpeedTest(rngtype &rng, size_t &count, double &duration_secs)
{
	std::chrono::high_resolution_clock clock;
	std::chrono::high_resolution_clock::time_point tstart;
	using namespace std::chrono_literals;
	std::chrono::high_resolution_clock::duration dT{100ms};

	constexpr size_t blockSize = 1e+5;
	count = 0;
	tstart = clock.now();
	while(clock.now() < tstart + dT)
    {
		//this cycle can be eliminated while compiler optimization if rng.state is not used after this
		for(size_t n = 0; n < blockSize; n++)
			rng();
        count += blockSize;
    }
	std::cout<<"rng state after test: "<<rng.getState()<<std::endl; //We need to read rng.state to avoid optimization mentioned above

	std::chrono::duration<double> duration = clock.now() - tstart;
	duration_secs = 1.0e-3*std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

TEST(speed, LCG)
{
    tea_MINSTD rng1;
    tea_MINSTD2 rng2;
	double dT1, dT2;
	size_t count1, count2;
	SpeedTest(rng1, count1, dT1);
	SpeedTest(rng2, count2, dT2);

	std::cout<<"Speed testing results:" <<std::endl;
	std::cout<<"LongInt<Int>::next :  " <<float(count1)/dT1<<" iters per sec"<<std::endl;
	std::cout<<"uint64_t::next  :     " <<float(count2)/dT2<<" iters per sec"<<std::endl;
}
