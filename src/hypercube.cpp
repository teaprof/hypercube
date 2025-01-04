#include "stat_tests/chi2based/StatisticalTestBase.h"
#include "progress/tictoc.h"
#include "stat_tests/hypercube/HypercubeTest.h"
#include "TestResultsDB/TestResultsDB.h"
#include "TestResultsDB/TaskDB.h"
#include "options/RngOptions.h"
#include "options/HypercubeOptions.h"
#include "options/ModeOptions.h"
#include "options/IOoptions.h"
#include<cassert>
#include<iostream>
#include "options/BasicOptions.h"

#include<memory>

class ProgramOptions : public Options {
    public:
    ProgramOptions() {
        addGroup(meta_data_options);
        addGroup(rng_options);
        addGroup(bits_repack_options);
        //addGroup(sampling_options);
        addGroup(hypercube_options);
        addGroup(subtask_options);
        addGroup(multithread_options);        
        addGroup(io_options);
        addGroup(basic_options);
    }
    MetaDataOptions meta_data_options;
    RNGOptions rng_options;
    BitsRepackOptions bits_repack_options;
    //SamplingOptions sampling_options;
    HypercubeOptions hypercube_options;
    SubtaskOptions subtask_options;
    MultithreadOptions multithread_options;
    IOOptions io_options;
    BasicOptions basic_options;
};

std::optional<TaskRecord> loadById(const ProgramOptions& options) {
    if(options.meta_data_options.taskId) {
        TaskDB taskDB;
        assert(options.io_options.inputFileName);
        taskDB.readFromFile(*options.io_options.inputFileName);
        return taskDB.find(*options.meta_data_options.taskId);
    }
    return std::nullopt;
}


int main(int argc, char* argv[]) {
    try {                
        ProgramOptions options;
        if(options.basic_options.needHelp()) {
            options.help();
            return 0;
        }        
        std::optional<MetaData> meta;
        HypercubeProblem problem;
        auto task = loadById(options);
        SubtaskParameters subtask;
        if(task) {
            meta = task->meta;
            problem = task->problem;
            subtask = task->subtask;
            //sampler = std::make_shared<HypercubeSampler>(...);
        } else {
            std::cout<<"rng = "<<options.rng_options.name<<std::endl;
            std::cout<<"threads = "<<options.multithread_options.nThreads()<<std::endl;
            std::cout<<"dim = "<<options.hypercube_options.dim<<std::endl;
            std::cout<<"nIntervals = "<<options.hypercube_options.nIntervals<<std::endl;
            std::cout<<"nSamples = "<<options.hypercube_options.nSamples<<std::endl;
            problem = createHypercubeProblem(options.hypercube_options);
            subtask = SubtaskParameters{.Ntasks=options.subtask_options.nSubtasks,.cur_task=options.subtask_options.curSubtask,.n_threads=options.multithread_options.nThreads()};
        }
        

        
        auto rng = createGenerator(options.rng_options, options.bits_repack_options);         
        //Chi2BasedTest<Histogram> test;
        Chi2BasedTest<HistogramAtomic> test;
        auto sampler = createHypercubeSampler(problem);
        //Chi2BasedTest<HistogramMutexed> test;
        
        tic();
        auto res = test.run(problem, subtask, sampler, rng->copy());
        double t1 = toc();        
        tic();
        std::cout<<"nthreads = "<<subtask.n_threads<<std::endl;
        std::cout<<"sum = "<<res.sum<<", sum2 = "<<res.sum2<<std::endl;
        std::cout<<"elapsed: "<<t1*1000<<" ms"<<std::endl;

        //TestResultsDB results_db;
        //results_db.readFromFile("data.json");
        //results_db.push_back(std::nullopt, problem, subtask, res);
        //results_db.sanitize();
        //results_db.writeToFile("data.json");
        return 0;
    } catch (std::runtime_error& r) {
        std::cerr<<"Error:"<<std::endl;
        std::cerr<<r.what()<<std::endl;
        return 1;
    }
}