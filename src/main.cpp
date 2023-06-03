#include <iostream>
#include <random>
#include <bitset>
#include <boost/program_options.hpp>
#include "hypercubetest.h"
#include "randombitgenerator.h"

using namespace std;


int main(int argc, char* argv[])
{
    boost::program_options::
/*  std::mt19937 mt;
    using RandomBitGenerator = stdRandomNumberEngine<decltype(mt)>;
    RandomBitGenerator randbitgen(mt);
    HypercubeTest<RandomBitGenerator> ht(1, 1000000, 2);
    std::cout<<"res = "<<ht(randbitgen)<<std::endl;
    return 0;*/

    std::mt19937 mt;
    using RandomBitGenerator = CaterpillarBitGenerator<std::mt19937>;
    RandomBitGenerator randbitgen(mt);
    BinaryHypercubeTest<RandomBitGenerator> ht(4, 1000000);
    std::cout<<"res = "<<ht(randbitgen)<<std::endl;
    return 0;
}
