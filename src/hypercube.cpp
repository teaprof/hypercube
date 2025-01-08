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

std::vector<TaskRecord> load(const ProgramOptions& options) {
    std::vector<TaskRecord> res;
    if(options.meta_data_options.taskId || options.meta_data_options.runall) {
        TaskDB taskDB;
        assert(options.io_options.inputFileName);
        taskDB.readFromFile(*options.io_options.inputFileName);
        if(options.meta_data_options.taskId) {
            auto task = taskDB.find(*options.meta_data_options.taskId);
            if(!task) {
                std::stringstream str;
                str<<"Input file "<<*options.io_options.inputFileName<<" doesn't contain task with taskid = "<<*options.meta_data_options.taskId;
                throw std::runtime_error(str.str());
            }
            res.push_back(*task);
        } else {
            assert(options.meta_data_options.runall);
            res = taskDB.records();
            if(res.empty()) {
                std::cout<<"The specified file doesn't contain any task"<<std::endl;
            }
        }
    }
    return res;
}

TaskRecord fromCmdLine(ProgramOptions& options) {
    TaskRecord res;
    res.problem = options.hypercube_options.problem();
    res.subtask = SubtaskParameters{.Ntasks=options.subtask_options.nSubtasks,.cur_task=options.subtask_options.curSubtask};
    res.repack = options.bits_repack_options.description();
    res.rng = options.rng_options.description();        
    return res;
}

void printProblem(const TaskRecord& task) {
    std::cout<<"rng id = "<<task.rng.rng_id<<std::endl;
    if(task.rng.seed) {
        std::cout<<"rng seed = "<<*task.rng.seed<<std::endl;
    } else {
        std::cout<<"rng seed = default "<<std::endl;
    }
    std::cout<<"rng offset = "<<task.rng.offset<<std::endl;
    if(task.repack) {
        std::cout<<"repack sample size = "<<task.repack->bits_per_sample<<std::endl;
        std::cout<<"repack srcLittleEndian = "<<task.repack->src_little_endian<<std::endl;
        std::cout<<"repack dstLittleEndian = "<<task.repack->dst_little_endian<<std::endl;
    } else {
        std::cout<<"no bits repack is used"<<std::endl;
    }
    std::cout<<"dim = "<<task.problem.dim<<std::endl;
    std::cout<<"nIntervals = "<<task.problem.m_intervals_per_dim<<std::endl;
    std::cout<<"nIntervalsTotal = "<<task.problem.m_intervals_total<<std::endl;
    std::cout<<"nSamples = "<<task.problem.N<<std::endl;

    std::cout<<"nSubtask = "<<task.subtask.Ntasks<<std::endl;
    std::cout<<"current subtask = "<<task.subtask.cur_task<<std::endl;
}


std::pair<std::vector<SubtaskResults>, std::vector<double>> run(const std::vector<TaskRecord>& tasks, ProgramOptions& options) {
    std::vector<SubtaskResults> res;
    std::vector<double> times;

    size_t nThreads = options.multithread_options.nThreads();

    for(auto task : tasks) {
        auto rng = createGenerator(task.rng, task.repack); 
        auto sampler = std::make_shared<HypercubeSampler>(task.problem);
        Chi2BasedTest<HistogramAtomic> test;
        
        tic();
        auto cur_res = test.run(task.problem, task.subtask, sampler, rng->copy(), nThreads);
        double time = toc();        
        tic();
        std::cout<<"sum = "<<cur_res.sum<<", sum2 = "<<cur_res.sum2<<std::endl;
        std::cout<<"elapsed: "<<time*1000<<" ms"<<std::endl;
        res.push_back(cur_res);
        times.push_back(time);
    }
    return {res, times};
}

std::vector<TaskRecord> init(ProgramOptions& options) {
    auto res = load(options);
    if(res.empty()) {
        res.push_back(fromCmdLine(options));
    }
    for(auto it : res)
        printProblem(it);
    return res;
}

void done(const std::vector<TaskRecord>& tasks, const std::vector<SubtaskResults>& results, const std::vector<double> times, ProgramOptions &options) {
    if(options.io_options.outputFileName) {
        const std::string& filename = *(options.io_options.outputFileName);
        TestResultsDB results_db; 
        if(std::filesystem::exists(filename)) {
            boost::interprocess::file_lock flock(filename.c_str());
            results_db.readFromFile(filename);
            results_db.add(tasks, results, times);
            results_db.writeToFile(filename);
        } else {
            results_db.add(tasks, results, times);
            results_db.writeToFile(filename);
        }
    }

}

int main(int argc, char* argv[]) {
    try {                
        ProgramOptions options;
        options.parse(argc, argv);
        if(options.basic_options.needHelp()) {
            options.help();
            return 0;
        }
        auto tasks = init(options);
        auto [results, times] = run(tasks, options);
        done(tasks, results, times, options);
        return 0;
    } catch (std::runtime_error& r) {
        std::cerr<<"Error:"<<std::endl;
        std::cerr<<r.what()<<std::endl;
        return 1;
    }
}