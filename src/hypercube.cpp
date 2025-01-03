#include "stat_tests/chi2based/StatisticalTestBase.h"
#include "progress/tictoc.h"
#include "stat_tests/hypercube/HypercubeTest.h"
#include "TestResultsDB/TestResultsDB.h"
#include "options/RngOptions.h"
#include "options/HypercubeOptions.h"
#include<cassert>
#include<iostream>
#include "options/BasicOptions.h"

#include<memory>

int main(int argc, char* argv[]) {
    try {        
        BasicOptions basic_options;
        RNGOptions rng_options;
        BitsRepackOptions bits_repack_options;
        //SamplingOptions sampling_options;
        MultithreadOptions multithread_options;
        HypercubeOptions hypercube_options;
        SubtaskOptions subtask_options;
        
        Options options;
        options.addGroup(rng_options);
        options.addGroup(bits_repack_options);
        //options.addGroup(sampling_options);
        options.addGroup(hypercube_options);        
        options.addGroup(multithread_options);        
        options.addGroup(subtask_options);
        options.addGroup(basic_options);
        options.parse(argc, argv);
        if(basic_options.need_help) {
            options.help();
            return 0;
        }
        options.validate();
        std::cout<<"rng = "<<rng_options.name<<std::endl;
        std::cout<<"threads = "<<multithread_options.nThreads()<<std::endl;
        std::cout<<"dim = "<<hypercube_options.dim<<std::endl;
        std::cout<<"nIntervals = "<<hypercube_options.nIntervals<<std::endl;
        std::cout<<"nSamples = "<<hypercube_options.nSamples<<std::endl;
        
        auto rng = createGenerator(rng_options, bits_repack_options);        
        auto problem = createHypercubeProblem(hypercube_options);
        auto sampler = createHypercubeSampler(problem);
        SubtaskParameters subtask{.Ntasks=subtask_options.nSubtasks,.cur_task=subtask_options.curSubtask,.n_threads=multithread_options.nThreads()};
        //Chi2BasedTest<Histogram> test;
        Chi2BasedTest<HistogramAtomic> test;
        //Chi2BasedTest<HistogramMutexed> test;
        
        tic();
        auto res = test.run(problem, subtask, sampler, rng->copy());
        double t1 = toc();        
        tic();
        std::cout<<"nthreads = "<<subtask.n_threads<<std::endl;
        std::cout<<"sum = "<<res.sum<<", sum2 = "<<res.sum2<<std::endl;
        std::cout<<"elapsed: "<<t1*1000<<" ms"<<std::endl;

        TestResultsDB results_db;
        results_db.readFromFile("data.json");
        results_db.push_back(problem, subtask, res);
        results_db.sanitize();
        results_db.writeToFile("data.json");
        return 0;
    } catch (std::runtime_error& r) {
        std::cerr<<"Error:"<<std::endl;
        std::cerr<<r.what()<<std::endl;
        return 1;
    }
}