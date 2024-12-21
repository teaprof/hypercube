/*
 * caterpillar.cpp
 *
 *  Created on: Apr 20, 2019
 *      Author: tea
 */

#include <a10engines/lcg.h>
#include "caterpillar.h"

#include<gtest/gtest.h>
#include<iostream>


using namespace mctools;


TEST(caterpillar, caterpillar)
{
	MINSTD gen;
	caterpillar<double, MINSTD> c(3, 2);
	c(gen);
	for(size_t n = 0; n < 10; n++)
	{
		double x1 = c[1];
		double x2 = c[2];
		c(gen);
		ASSERT_EQ(c[0], x2);
	}

}
