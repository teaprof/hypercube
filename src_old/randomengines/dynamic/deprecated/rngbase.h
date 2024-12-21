/*
 * rngbase.h
 *
 *  Created on: May 21, 2019
 *      Author: tea
 */

#ifndef A30HIERARCHY_RNGBASE_H_
#define A30HIERARCHY_RNGBASE_H_

#include<iostream>
#include<stdint>
#include"a10random/a10static/a10uniformrandombitgenerators/tea/lcg.h"

namespace mctools {

/*!
 * \brief The base class for all classes that wrap random engines from other libraries
 */
class rngbase
{
public:
    typedef uintmax_t counter_type;
    rngbase();
    virtual ~rngbase();

    virtual uintmax_t count();

    //Following declarations make this class meet the uniform random bit generator requirements
    using result_type = uintmax_t;
    virtual result_type operator()() = 0;
    virtual result_type min() = 0;
    virtual result_type max() = 0;

    virtual double getDouble() = 0;
    virtual float getFloat() = 0;


    //virtual std::ostream &operator<<(std::ostream& stream) = 0;
    //virtual std::istream &operator>>(std::istream& stream) = 0;
protected:
    counter_type mcount;
};

template<class RNG>
class rng_wrap : public rngbase
{
    RNG mLCG;
public:
    //Following declarations make this class meet the uniform random bit generator requirements
    result_type operator()() override
    {
        return mLCG();
    }
    virtual result_type min() override
    {
        mcount++;
        return mLCG.min();
    }
    virtual result_type max() override
    {
        mcount++;
        return mLCG.max();
    }
    double getDouble()
    {

    }
    float getFloat()
    {

    }
};

typedef rng_wrap<MINSTD> rng_MINSTD;
typedef rng_wrap<MINSTD2> rng_MINSTD2;

} /* namespace mctools */


#endif /* A30HIERARCHY_RNGBASE_H_ */
