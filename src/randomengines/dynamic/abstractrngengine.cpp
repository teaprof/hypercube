#include "abstractrngengine.h"

#include "static/uniformrandombitgenerators/stdwrap/stdrng.h"
#include "static/uniformrandombitgenerators/tea/lcg.h"

using namespace mctools;

AbstractRNGEngine::AbstractRNGEngine()
{
    //nothing to do
}
AbstractRNGEngine::~AbstractRNGEngine()
{
    //nothing to do
}
void AbstractRNGEngine::discard()
{
    (*this)();
}
void AbstractRNGEngine::discard(size_t count)
{
    for(size_t a = count; a > 0; a--)
        discard();
}

double AbstractRNGEngine::getDouble()
{
	return static_cast<double>((*this)() - min())/static_cast<double>(max() - min());
}
float AbstractRNGEngine::getFloat()
{
	return static_cast<float>((*this)() - min())/static_cast<float>(max() - min());
}


RNGFactory::RNGFactory()
{
    init();
}

std::shared_ptr<AbstractRNGEngine> RNGFactory::create(size_t rngID)
{
   switch(rngID)
   {
   case 0: return std::make_shared<UniformBitsEngine<std_minstd_rand>>();
   case 1: return std::make_shared<UniformBitsEngine<std_minstd_rand0>>();
   case 2: return std::make_shared<UniformBitsEngine<std_mt19937>>();
   case 3: return std::make_shared<UniformBitsEngine<std_mt19937_64>>();
   case 4: return std::make_shared<UniformBitsEngine<std_ranlux24>>();
   case 5: return std::make_shared<UniformBitsEngine<std_ranlux24_base>>();
   case 6: return std::make_shared<UniformBitsEngine<std_ranlux48>>();
   case 7: return std::make_shared<UniformBitsEngine<std_ranlux48_base>>();
   case 8: return std::make_shared<UniformBitsEngine<std_knuth_b>>();
   case 9: return std::make_shared<UniformBitsEngine<tea_minstd_rand0>>();
   case 10: return std::make_shared<UniformBitsEngine<tea_minstd_rand>>();
   case 11: return std::make_shared<UniformBitsEngine<tea_MINSTD>>();
   case 12: return std::make_shared<UniformBitsEngine<tea_MINSTD2>>();
   default: throw std::runtime_error(std::string("Unknown rng type: ") + std::to_string(rngID));
   }
}

std::shared_ptr<AbstractRNGEngine> RNGFactory::create(const std::string& name)
{
    auto it = names.find(name);
    if(it == names.end())
        throw std::runtime_error("unknown name");
    return create(it->second);
}

size_t RNGFactory::count()
{
    return names.size();
}
const std::string& RNGFactory::getname(size_t idx)
{
    for(const auto& it : names)
        if(it.second == idx)
            return it.first;
    throw std::runtime_error("unknown generator");
}

bool RNGFactory::init()
{
    names["std_minstd_rand"] = 0;
    names["std_minstd_rand0"] = 1;
    names["std_mt19937"] = 2;
    names["std_mt19937_64"] = 3;
    names["std_ranlux24"] = 4;
    names["std_ranlux24_base"] = 5;
    names["std_ranlux48"] = 6;
    names["std_ranlux48_base"] = 7;
    names["std_knuth_b"] = 8;
    names["tea_minstd_rand0"] = 9;
    names["tea_minstd_rand"] = 10;
    names["tea_MINSTD"] = 11;
    names["tea_MINSTD2"] = 12;
    return true;
}
