#ifndef TICTOC_H
#define TICTOC_H
#include<chrono>

struct Timer {    
    using TClock = std::chrono::high_resolution_clock;
    typename TClock::time_point tstart;

    void tic();
    double toc();
};


void tic();
double toc();

#endif