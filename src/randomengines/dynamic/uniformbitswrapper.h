/*
 * RNGFactory.h
 *
 *  Created on: May 21, 2019
 *      Author: tea
 */

#ifndef A30HIERARCHY_URBGWRAPPER_H_
#define A30HIERARCHY_URBGWRAPPER_H_

#include "static/rngconcept.h"

#include<memory>
#include<list>
#include<map>

namespace mctools {

enum rngtype
{
    rngtypeMINSTD = 0,
    rngtypeMINSTD2
};

class UniformBitGeneratorBase
{
public:
    using inttype = uintmax_t;
    UniformBitGeneratorBase ();
    virtual ~UniformBitGeneratorBase () {};

    virtual inttype min();
    virtual inttype max();
    virtual inttype operator()();
};

template<uint_random_generator RNG>
class UniformBitGeneratorWrapper : public UniformBitGeneratorBase
//This class is not used, use URBGEngine instead of it
{
public:
    static_assert (sizeof(typename RNG::result_type) <= sizeof(inttype), "Increase sizeof UniformBitGeneratorBase::inttype");

    UniformBitGeneratorWrapper ()
    { /* nothing to do */ };
    UniformBitGeneratorWrapper (const RNG& rng_) : rng(rng_)
    { /* nothing to do */ };
    UniformBitGeneratorWrapper (RNG&& rng_) : rng(std::move(rng_))
    { /* nothing to do */ };

    virtual ~UniformBitGeneratorWrapper ();

    virtual inttype min()
    {
        return RNG::min();
    }
    virtual inttype max()
    {
        return RNG::max();
    }
    virtual inttype operator()()
    {
        return rng();
    }
protected:
    RNG rng;
};


} /* namespace mctools */

#endif /* A30HIERARCHY_URBGWRAPPER_H_ */
