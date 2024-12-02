#include "StatisticalTestBase.h"
#include<cassert>
#include<chrono>
#include<iostream>

auto tstart = std::chrono::high_resolution_clock::now();

void tic() {
    tstart = std::chrono::high_resolution_clock::now();
}

void toc() {
    auto tend = std::chrono::high_resolution_clock::now();

    auto period = std::chrono::high_resolution_clock::period();
    double dt = static_cast<double>(period.num)/period.den;
    double t = (tend - tstart).count()*dt;
    std::cout<<"t "<<t*1000<<" ms"<<std::endl;
}


int main() {
    HypercubeTestParameters task(2, 100, 1000000);
    SubtaskParameters subtask{.Ntasks=1,.cur_task=0,.n_threads=1};
    Chi2BasedTest test;
    MT19937Wrapper rng;
    KIndependentGenerator sampler(task.dim, task.m_intervals_per_dim);
    tic();
    auto res = test.run<MT19937Wrapper, KIndependentGenerator>(task, subtask, rng, sampler);
    toc();
    return 0;
}