#include "StatisticalTestBase.h"
#include "progress/tictoc.h"
#include<cassert>
#include<iostream>
#include "options/BasicOptions.h"

int main(int arcg, char* argv[]) {
    /*try {        
        HypercubeOptions options(argc, argv);
        if(options.needHelp)
        {
            options.help();
            return 0;
        }
        options.validate();
        return 0;
    } catch (std::runtime_error& r) {
        std::cerr<<"Error:"<<std::endl;
        std::cerr<<r.what()<<std::endl;
        return 1;
    }*/
    
    HypercubeTestParameters task(3, 100, 200000000);
    SubtaskParameters subtask{.Ntasks=1,.cur_task=0,.n_threads=10};
    Chi2BasedTest<Histogram> test;
    Chi2BasedTest<HistogramAtomic> test_atomic;
    Chi2BasedTest<HistogramMutexed> test_mutexed;
    MT19937Wrapper rng;
    KIndependentGenerator sampler(task.dim, task.m_intervals_per_dim);
    tic();
    auto res1 = test.run<MT19937Wrapper, KIndependentGenerator>(task, subtask, rng, sampler);    
    double t1 = toc();
    tic();
    auto res2 = test_atomic.run<MT19937Wrapper, KIndependentGenerator>(task, subtask, rng, sampler);    
    double t2 = toc();
    tic();
    auto res3 = test_mutexed.run<MT19937Wrapper, KIndependentGenerator>(task, subtask, rng, sampler);    
    double t3 = toc();
    std::cout<<"nthreads = "<<subtask.n_threads<<std::endl;
    std::cout<<"Simple: "<<res1.sum<<" "<<res1.sum2<<"; elapsed = "<<t1*1000<<" ms"<<std::endl;
    std::cout<<"Atomic: "<<res2.sum<<" "<<res2.sum2<<"; elapsed = "<<t2*1000<<" ms"<<std::endl;
    std::cout<<"Mutexs: "<<res3.sum<<" "<<res3.sum2<<"; elapsed = "<<t3*1000<<" ms"<<std::endl;    
    return 0;
}