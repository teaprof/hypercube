/*
 * caterpillar.h
 *
 *  Created on: Apr 20, 2019
 *      Author: tea
 */

//todo: Use 'random number sequence stride' instead of 'caterpillar'

#ifndef a10RNG_CATERPILLAR_H_
#define a10RNG_CATERPILLAR_H_

#include<cstddef>
#include<vector>
#include<boost/circular_buffer.hpp>
#include<boost/assert.hpp>

#include "../a20uniform/standardrng.h"


namespace mctools
{

template<class valueType, class generator>
class caterpillar
{
public:
	size_t dimension;
	size_t step;
	boost::circular_buffer<valueType> cb;
	caterpillar(size_t dimension, size_t step)
	{
		this->dimension = dimension;
		this->step = step;
		cb.set_capacity(dimension);
	}
	virtual ~caterpillar()
	{
		//nothing to do
	}
	boost::circular_buffer<double> operator()(generator &gen)
	{
		for(size_t k = 0; k < step; k++)
			cb.push_back(gen());
		//this can occur on the first run when we need totally initialize cb
		while(cb.size() < dimension)
			cb.push_back(gen());
		return cb;
	}
	double operator[](size_t Index)
	{
		BOOST_ASSERT_MSG(cb.size() == dimension, "caterpillar::operator() should be called before calling caterpillar::operator[]");
		return cb[Index];
	}
};


class chain : public caterpillar<double, rngU01base>
{
	std::shared_ptr<rngU01base> mrng;
public:
	chain(size_t dimension, size_t step, std::shared_ptr<rngU01base> rng) :
		caterpillar(dimension, step), mrng(rng)
	{
		this->dimension = dimension;
		this->step = step;
		cb.set_capacity(dimension);
	}
	virtual ~chain()
	{
		//nothing to do
	}

	boost::circular_buffer<double> operator()()
	{
		return caterpillar<double, rngU01base>::operator()(*mrng);
	}
};



} /* namespace mctools */

#endif /* 10RNG_CATERPILLAR_H_ */

