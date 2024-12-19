#include "StatisticalTestBase.h"
#include "progress/tictoc.h"
#include "HypercubeTest.h"
#include<cassert>
#include<iostream>
#include "options/BasicOptions.h"

int main(int argc, char* argv[]) {
    try {        
        BasicOptions basic_options;
        MultithreadOptions multithread_options;
        HypercubeOptions hypercube_options;
        SubtaskOptions subtask_options;
        ChainOptions chain_options;
        SampleToIntOptions index_options;
        BitsRepackOptions bits_repack_options;
        
        Options options;
        options.addGroup(hypercube_options);
        options.addGroup(multithread_options);        
        options.addGroup(subtask_options);
        options.addGroup(basic_options);
        options.addGroup(chain_options);
        options.addGroup(index_options);
        options.addGroup(bits_repack_options);
        options.parse(argc, argv);
        if(basic_options.need_help) {
            options.help();
            return 0;
        }
        options.validate();
        std::cout<<"threads = "<<multithread_options.nThreads()<<std::endl;
        std::cout<<"dim = "<<hypercube_options.dim<<std::endl;
        std::cout<<"nIntervals = "<<hypercube_options.nIntervals<<std::endl;
        std::cout<<"nSamples = "<<hypercube_options.nSamples<<std::endl;
   
        HypercubeProblem problem(hypercube_options.dim, hypercube_options.nIntervals, hypercube_options.nSamples);
        SubtaskParameters subtask{.Ntasks=subtask_options.nSubtasks,.cur_task=subtask_options.curSubtask,.n_threads=multithread_options.nThreads()};
        Chi2BasedTest<Histogram> test;
        Chi2BasedTest<HistogramAtomic> test_atomic;
        Chi2BasedTest<HistogramMutexed> test_mutexed;
        MT19937Wrapper rng;
        BitsRepack bits_generator(bits_repack_options.sampleBits, bits_repack_options.littleEndian, index_options.littleEndian);
        UniformIntDistribution int_generator;
        HypercubeIntSampler hypercube_int_sampler(problem, chain_options.stride);
        
        tic();
        auto res1 = test.run(problem, subtask, hypercube_int_sampler, rng);
        double t1 = toc();
        tic();
        auto res2 = test_atomic.run(problem, subtask, hypercube_int_sampler, rng);
        double t2 = toc();
        tic();
        auto res3 = test_mutexed.run(problem, subtask, hypercube_int_sampler, rng);
        double t3 = toc();
        std::cout<<"nthreads = "<<subtask.n_threads<<std::endl;
        std::cout<<"Simple: "<<res1.sum<<" "<<res1.sum2<<"; elapsed = "<<t1*1000<<" ms"<<std::endl;
        std::cout<<"Atomic: "<<res2.sum<<" "<<res2.sum2<<"; elapsed = "<<t2*1000<<" ms"<<std::endl;
        std::cout<<"Mutexs: "<<res3.sum<<" "<<res3.sum2<<"; elapsed = "<<t3*1000<<" ms"<<std::endl;    
        return 0;
    } catch (std::runtime_error& r) {
        std::cerr<<"Error:"<<std::endl;
        std::cerr<<r.what()<<std::endl;
        return 1;
    }
}