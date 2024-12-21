#ifndef INTTOU01_H
#define INTTOU01_H

#include<random>
#include"static/rngconcept.h"

namespace mctools {

template<std::uniform_random_bit_generator RNG, std::floating_point Float, Float a, Float b>
class IntToUab
{
public:
    double operator()(RNG& rng)
    {
        auto val = rng() - RNG::min();
        return a + (b - a)*val/(RNG::max() - RNG::min());
    }
    static constexpr Float min()
    {
        return a;
    }
    static constexpr Float max()
    {
        return b;
    }
};

template<class RNG, class Float>
using IntToU01 = IntToUab<RNG, Float, 0, 1>;


/*!
 * \brief Converts real value from [0, 1) to [Min, Min+1, ..., Max]
 */
class U01toInt {
    int mMin;
    int mMax;
public:
    U01toInt(int Min, int Max);
    virtual ~U01toInt();

    int operator()(double val);
};

} /* namespace mctools */

#endif // INTTOU01_H
