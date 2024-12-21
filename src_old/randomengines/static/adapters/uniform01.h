#ifndef U01_H
#define U01_H

#include<inttypes.h>
#include<type_traits>

namespace mctools
{

//generate doubles from [0, 1]
//bytes -- сколько байт использовать для создания одного числа формата double
//Представляет собой надстройку над uniform bit generator, позволяющую преобразовать его отчёты к формату double
//При этом для заполнения всех бит числа double расходуется столько случайных чисел, сколько надо.
template<class RNG, int bytes>
class rngU01
{
    using Int = typename RNG::result_type;
    constexpr static uint8_t IntSize = sizeof(int);
public:
    template<class T = RNG>
    typename std::enable_if<bytes <= sizeof(typename T::result_type), double>::type
    getdouble(RNG& rng)
    {
        Int res = rng();
        double range = rng.max() - rng.min();
        double a = double(res - rng.min())/range;
        return a;
    }

    //todo: вот здесь хорошо было бы указать диапазон sizeof(typename T::result_type) < bytes <= sizeof(typename T::result_type)
    template<class T = RNG>
    typename std::enable_if<bytes == 2*sizeof(typename T::result_type), double>::type
    getdouble(RNG& rng)
    {
        Int res1 = rng();
        Int res2 = rng();
        double range = rng.max() - rng.min();
        double a1 = double(res1 - RNG::min())/range;
        double a2 = double(res2 - RNG::min())/range;
        return (a2 + a1/range)/range;
    }


    template<class T = RNG>
    typename std::enable_if<bytes == 4*sizeof(typename T::result_type), double>::type
    getdouble(RNG& rng)
    {
        Int res1 = rng();
        Int res2 = rng();
        Int res3 = rng();
        Int res4 = rng();
        double range = RNG::maxValue - RNG::minValue;
        double a1 = double(res1 - RNG::minValue)/range;
        double a2 = double(res2 - RNG::minValue)/range;
        double a3 = double(res1 - RNG::minValue)/range;
        double a4 = double(res2 - RNG::minValue)/range;
        return (a4+(a3 + (a2 + a1/range)/range)/range)/range;
    }
    double operator()(RNG& rng)
    {
        return getdouble(rng);
    }
};

} /* namespace mctools */

#endif // U01_H
