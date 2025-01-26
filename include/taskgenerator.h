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
        addGroup(subtasks_output_options_);
        addGroup(runtime_options_);
    }
    private:
    SubtasksOutput subtasks_output_options_;
    RuntimeOptions runtime_options_;
};


class TaskGenerator {
    public:
    void run() {
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
        return 0;
    }
}