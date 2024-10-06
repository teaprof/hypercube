/*
 * standardrng.cpp
 *
 *  Created on: Apr 20, 2019
 *      Author: tea
 */
#include "../../adapters/deprecated/standardrng.h"

#include "a10random/a10static/a10uniformrandombitgenerators/tea/lcg.h"
#include<gtest/gtest.h>
#include<cstddef>
#include<iostream>

using namespace mctools;


TEST(RNGU01, StandardRNG)
{
	rngU01<MINSTD, 2> U01_2;
	rngU01<MINSTD, 4> U01_4;
	rngU01<MINSTD, 8> U01_8;
	rngU01<MINSTD, 16> U01_16;
	double minx = 1;
	for(size_t n = 0; n < 1e+5; n++)
	{
		double x = U01_8();
		if(x > 0 && x < minx)
			minx = x;
		ASSERT_GE(x, 0);
		ASSERT_LE(x, 1);
	}
	std::cout<<"minx = "<<minx<<std::endl;
}





