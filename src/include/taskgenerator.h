#include "stat_tests/chi2based/StatisticalTestBase.h"
#include "stat_tests/hypercube/HypercubeTest.h"
#include "TestResultsDB/SubtaskDB.h"
#include "options/RngOptions.h"
#include "options/HypercubeOptions.h"
#include "options/ProgramOptions.h"
#include "options/IOoptions.h"
#include<cassert>
#include<iostream>
#include "options/TaskGeneratorOptions.h"
#include <boost/interprocess/sync/file_lock.hpp>
#include <hash/hash.h>

#include<memory>

class TaskGeneratorOptions : public ProgramOptions {
    public:
    TaskGeneratorOptions() {
        subtasks_output_options = std::make_shared<SubtasksOutput>();
        hypercube_options = std::make_shared<HypercubeOptions>();
        runtime_options = std::make_shared<RuntimeOptions>();
        rng_options = std::make_shared<RNGOptions>();
        addGroup(subtasks_output_options);
        addGroup(runtime_options);
        addGroup(rng_options);
        addGroup(hypercube_options); // to add a specified hypercube test 
    }
    std::shared_ptr<SubtasksOutput> subtasks_output_options;
    std::shared_ptr<HypercubeOptions> hypercube_options;
    std::shared_ptr<RuntimeOptions> runtime_options;
    std::shared_ptr<RNGOptions> rng_options;
};


class TaskGenerator {
    public:
    TaskGenerator(const std::shared_ptr<TaskGeneratorOptions>& options) : options_{options} {}
    void init() {}
    void run() {
        size_t subtask_id = 0;
        size_t task_id = 0;
        SubtaskDB tasks;
        size_t filter_by_total_points_count = 0;
        size_t filter_by_subtasks = 0;
        for(auto problem : *options_->hypercube_options) {
            double totalMB = problem.n_cells_total*sizeof(size_t)/1024.0/1024.0;
            size_t total_subtasks = std::ceil(totalMB/options_->runtime_options->max_ram_MB);
            if(problem.N >= options_->runtime_options->maxTotalPoints) {
                filter_by_total_points_count++;
                continue;
            }
            if(total_subtasks > options_->runtime_options->maxSubtasks) {
                filter_by_subtasks++;
                continue;
            }
            if(++task_id > options_->runtime_options->maxTasks) {
                std::cout<<"Reached maximum number of tasks"<<std::endl;
                break;
            }
            for(size_t n = 0; n < total_subtasks; n++) {
                SubtaskParameters subtask{.Ntasks = total_subtasks, .cur_task=n};
                //subtask.repack = options_->bits_repack_options->description();
                //subtask.rng = options_->rng_options->description();                        
                auto rng_descr = options_->rng_options->description();                
                auto hash = myhash(problem, subtask);
                MetaData meta1{.taskId=hash};
                tasks.push_back(meta1, rng_descr, std::nullopt, problem, subtask);
                if(tasks.records().size() > 100'000'000) {
                    throw std::runtime_error("The number of subtasks becomes greater than 100 millions");
                }
            }
        }
        std::cout<<"Totally generated "<<task_id<<" tasks ("<<tasks.records().size()<<" subtasks)"<<std::endl;
        std::cout<<"Filtered by maxTotalPoints filter "<<filter_by_total_points_count<<" tasks"<<std::endl;
        std::cout<<"Filtered by maxSubtask filter "<<filter_by_subtasks<<" tasks"<<std::endl;
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
    const std::shared_ptr<TaskGeneratorOptions>& options_;
};
