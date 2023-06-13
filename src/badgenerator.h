#ifndef BADGENERATOR_H
#define BADGENERATOR_H
#include <boost/program_options.hpp>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <iostream>

class BadGenerator
{
public:
    BadGenerator(size_t dim);
    float& operator[](size_t index);
    bool gen();
    char genbyte();
private:
    std::vector<float> buf;
    size_t index, maxindex;
    std::mt19937 rng;
};

#endif // BADGENERATOR_H
