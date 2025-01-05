#include "stat_tests/chi2based/StatisticalTestBase.h"
#include "progress/tictoc.h"
#include "stat_tests/hypercube/HypercubeTest.h"
#include "TestResultsDB/TestResultsDB.h"
#include "TestResultsDB/TaskDB.h"
#include "options/RngOptions.h"
#include "options/SubtaskOptions.h"
#include "options/HypercubeOptions.h"
#include "options/ModeOptions.h"
#include "options/IOoptions.h"
#include<cassert>
#include<iostream>
#include<filesystem>
#include "options/BasicOptions.h"
#include <boost/interprocess/sync/file_lock.hpp>

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
        auto res = taskDB.find(*options.meta_data_options.taskId);
        if(!res) {
            std::stringstream str;
            str<<"Input file "<<*options.io_options.inputFileName<<" doesn't contain task with taskid = "<<*options.meta_data_options.taskId;
            throw std::runtime_error(str.str());
        }
        return res;
    }
    return std::nullopt;
}

void printProblem(RandomNumberGeneratorDescription& rng_descr, std::optional<BitsRepackDescription> bitsrepack_descr, HypercubeProblem problem, SubtaskParameters subtask, size_t nThreads) {
    std::cout<<"rng id = "<<rng_descr.rng_id<<std::endl;
    if(rng_descr.seed) {
        std::cout<<"rng seed = "<<*rng_descr.seed<<std::endl;
    } else {
        std::cout<<"rng seed = default "<<std::endl;
    }
    std::cout<<"rng offset = "<<rng_descr.offset<<std::endl;
    if(bitsrepack_descr) {
        std::cout<<"repack sample size = "<<bitsrepack_descr->bits_per_sample<<std::endl;
        std::cout<<"repack srcLittleEndian = "<<bitsrepack_descr->src_little_endian<<std::endl;
        std::cout<<"repack dstLittleEndian = "<<bitsrepack_descr->dst_little_endian<<std::endl;
    } else {
        std::cout<<"no bits repack is used"<<std::endl;
    }
    std::cout<<"dim = "<<problem.dim<<std::endl;
    std::cout<<"nIntervals = "<<problem.m_intervals_per_dim<<std::endl;
    std::cout<<"nIntervalsTotal = "<<problem.m_intervals_total<<std::endl;
    std::cout<<"nSamples = "<<problem.N<<std::endl;

    std::cout<<"nSubtask = "<<subtask.Ntasks<<std::endl;
    std::cout<<"current subtask = "<<subtask.cur_task<<std::endl;

    std::cout<<"nThreads = "<<nThreads<<std::endl;
}


int main(int argc, char* argv[]) {
    try {                
        ProgramOptions options;
        options.parse(argc, argv);
        if(options.basic_options.needHelp()) {
            options.help();
            return 0;
        }        
        std::optional<MetaData> meta;
        RandomNumberGeneratorDescription rng_description;
        std::optional<BitsRepackDescription> bitsrepack_description;
        HypercubeProblem problem;
        SubtaskParameters subtask;        
        auto task = loadById(options);
        if(task) {
            meta = task->meta;
            rng_description = task->rng;
            bitsrepack_description = task->repack;
            problem = task->problem;
            subtask = task->subtask;            
        } else {
            problem = options.hypercube_options.problem();
            subtask = SubtaskParameters{.Ntasks=options.subtask_options.nSubtasks,.cur_task=options.subtask_options.curSubtask};
            bitsrepack_description = options.bits_repack_options.description();
            rng_description = options.rng_options.description();        
        } 
        size_t nThreads = options.multithread_options.nThreads();

        printProblem(rng_description, bitsrepack_description, problem, subtask, nThreads);
                
        auto rng = createGenerator(rng_description, bitsrepack_description); 
        //Chi2BasedTest<Histogram> test;
        Chi2BasedTest<HistogramAtomic> test;
        auto sampler = std::make_shared<HypercubeSampler>(problem);

        //Chi2BasedTest<HistogramMutexed> test;
        
        tic();
        auto res = test.run(problem, subtask, sampler, rng->copy(), nThreads);
        double time = toc();        
        tic();
        std::cout<<"sum = "<<res.sum<<", sum2 = "<<res.sum2<<std::endl;
        std::cout<<"elapsed: "<<time*1000<<" ms"<<std::endl;

        if(options.io_options.outputFileName) {
            const std::string& filename = *(options.io_options.outputFileName);
            TestResultsDB results_db;            
            if(std::filesystem::exists(filename)) {
                boost::interprocess::file_lock flock(filename.c_str());
                results_db.readFromFile(filename);
                results_db.push_back(meta, rng_description, bitsrepack_description, problem, subtask, res, time);
                results_db.sanitize();
                results_db.writeToFile(filename);
            } else {
                results_db.push_back(meta, rng_description, bitsrepack_description, problem, subtask, res, time);
                results_db.writeToFile(filename);
            }
        }

        return 0;
    } catch (std::runtime_error& r) {
        std::cerr<<"Error:"<<std::endl;
        std::cerr<<r.what()<<std::endl;
        return 1;
    }
}