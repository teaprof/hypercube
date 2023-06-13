#include "badgenerator.h"
//#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>


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
