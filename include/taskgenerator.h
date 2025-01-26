#include "stat_tests/chi2based/StatisticalTestBase.h"
#include "progress/tictoc.h"
#include "stat_tests/hypercube/HypercubeTest.h"
#include "TestResultsDB/SubtasksResultsDB.h"
#include "TestResultsDB/SubtaskDB.h"
#include "TestResultsDB/TaskResultsDB.h"
#include "options/RngOptions.h"
#include "options/SubtaskOptions.h"
#include "options/HypercubeOptions.h"
#include "options/MetaDataOptions.h"
#include "options/ProgramOptions.h"
#include "options/IOoptions.h"
#include "serializers/prettyserializer.h"
#include<cassert>
#include<iostream>
#include<filesystem>
#include "options/BasicOptions.h"
#include "options/TaskGeneratorOptions.h"
#include <boost/interprocess/sync/file_lock.hpp>

#include<memory>

class TaskGeneratorOptions : public ProgramOptions {
    public:
    TaskGeneratorOptions() {
        addGroup(subtasks_output_options);
        addGroup(runtime_options);
        addGroup(rng_options);
        addGroup(hypercube_options); // to add a specified hypercube test        
    }
    SubtasksOutput subtasks_output_options;
    HypercubeOptions hypercube_options;
    RuntimeOptions runtime_options;
    RNGOptions rng_options;
};


class TaskGenerator {
    public:
    TaskGenerator(const TaskGeneratorOptions& options) : options_{options} {}
    void init() {}
    void run() {
        size_t task_id = 0;
        SubtaskDB tasks;
        for(auto problem : options_.hypercube_options) {            
            size_t totalMB = problem.n_cells_total*sizeof(size_t);
            size_t total_subtasks = std::ceil(totalMB/options_.runtime_options.RAM/1024.0/1024.0);
            for(size_t n = 0; n < total_subtasks; n++) {
                SubtaskParameters subtask{.Ntasks = total_subtasks, .cur_task=n};
                //subtask.repack = options_.bits_repack_options.description();
                //subtask.rng = options_.rng_options.description();                        
                auto rng_descr = options_.rng_options.description();
                MetaData meta1{.taskId=task_id++};
                tasks.push_back(meta1, rng_descr, std::nullopt, problem, subtask);
            }                            
        }
        std::cout<<"Totally generated "<<tasks.records().size()<<" subtasks"<<std::endl;
        std::cout<<"Writing db..."<<std::endl;
        tasks.writeToFile("subtasks.json");
        std::cout<<"finished!"<<std::endl;
    }
    /*void run() {
        SubtaskDB tasks;
        size_t task_id = 0;
        for(size_t dim = 1; dim <= 3; dim++) {
            for(size_t stride = 1; stride <= dim; stride++) {
                SubtaskParameters subtask{.Ntasks=1,.cur_task=0};
                RandomNumberGeneratorDescription rng_descr{.rng_id=0,.offset=0};
                BitsRepackDescription bits_repack{.src_little_endian=true,.dst_little_endian=true,.bits_per_sample=32};
                size_t mIntervals = 100;
                size_t N = 1;
                for(size_t k = 0; k < dim; k++) {
                    N *= mIntervals;
                }
                N *= 100;
                HypercubeProblem problem(dim, mIntervals, stride, N);

                MetaData meta1{.taskId=task_id++};
                tasks.push_back(meta1, rng_descr, std::nullopt, problem, subtask);
                MetaData meta2{.taskId=task_id++};
                tasks.push_back(meta2, rng_descr, bits_repack, problem, subtask);
            }
        }
        tasks.writeToFile("subtasks.json");
    }*/
    void done() {

    }
    private:
    TaskGeneratorOptions options_;
};
