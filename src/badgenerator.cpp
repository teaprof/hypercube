#include "badgenerator.h"
//#include <boost/program_options.hpp>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <iostream>

BadGenerator::BadGenerator(size_t dim): rng(std::chrono::steady_clock::now().time_since_epoch().count())
{
    buf.assign(static_cast<size_t>(1) << dim, 0);
    for(size_t n = 0; n < buf.size(); n++)
        buf[n] = 0; // difference between probabilities of 0 and 1
    maxindex = buf.size() - 1;
    index = rng() & maxindex;
}
bool BadGenerator::gen() {
    size_t index0 = (index << 1) & maxindex;
    size_t index1 = index0 + 1;
    double p0 = 0.5 + buf[index0]/2.0;
    double p1 = 0.5 - buf[index1]/2.0;
    std::uniform_real_distribution<double> u01(0, 1);
    if(u01(rng) < p0/(p0+p1))
    {
        index = index0;
        return false;
    } else {
        index = index1;
        return true;
    }
}
char BadGenerator::genbyte()
{
    char res = 0;
    for(size_t i = 0; i < 8; i++)
        res = (res << 1) + char(gen());
    return res;
}
float& BadGenerator::operator[](size_t index) {
    return buf[index];
}

int main(int argc, char* argv[])
{
    BadGenerator bg(20);
    std::ofstream f("out.bin", std::ios::trunc | std::ios::binary);
    for(size_t n = 0; n < 512*1024*1024; n++)
    {
        char b = bg.genbyte();
        f.write(&b, 1);
    }
    return 0;
}
