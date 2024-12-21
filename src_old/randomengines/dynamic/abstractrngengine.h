#ifndef ABSTRACTRNGENGINE_H
#define ABSTRACTRNGENGINE_H

#include<cstdint>
#include<cstddef>
#include<map>
#include "../static/adapters/IntUniform.h"
#include"dynamic/uniformbitswrapper.h"

namespace mctools {

class AbstractRNGEngine
{
public:
    using result_type = uintmax_t;
    AbstractRNGEngine();
    virtual ~AbstractRNGEngine();

    virtual void discard();
    virtual void discard(size_t count);

    template<class Int>
    Int get(const Int& left, const Int& right)
    {
        return UniformIntSegment<AbstractRNGEngine, Int>::get(*this, left, right);
    }

    virtual result_type operator()() = 0;

    virtual result_type min() = 0;
    virtual result_type max() = 0;

    virtual double getDouble();
    virtual float getFloat();

    //add load
    //add save
    //add seed
};

template<uint_random_generator Gen>
class UniformBitsEngine : public AbstractRNGEngine
{
public:
    UniformBitsEngine()
    { /* nothing to do */ };
    UniformBitsEngine(const UniformBitsEngine<Gen>& val) : mURBG(val.mURBG)
    { /* nothing to do */ };
    UniformBitsEngine(UniformBitsEngine<Gen>&& val) : mURBG(std::move(val.mURBG))
    { /* nothing to do */ };
    ~UniformBitsEngine()
    { /* nothing to do */ };

    result_type operator()() override
    {
        return mURBG();
    }

    result_type min() override
    {
        return Gen::min();
    }
    result_type max() override
    {
        return Gen::max();
    }
protected:
    Gen mURBG;
};

class RNGFactory
{    
public:
    RNGFactory();
    std::shared_ptr<AbstractRNGEngine> create(size_t rngID);
    std::shared_ptr<AbstractRNGEngine> create(const std::string& name);
    bool init();

    size_t count();
    const std::string& getname(size_t idx);
private:
    std::map<std::string, size_t> names;
};


} /* namespace mctools */

#endif // ABSTRACTRNGENGINE_H
