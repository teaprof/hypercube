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
#include <boost/interprocess/sync/file_lock.hpp>

#include<memory>

class GatherOptions : public ProgramOptions {
    public:
    GatherOptions() {
        io_options = std::make_shared<IOOptions>("gatherer io options");
        addGroup(io_options);
    }
    GatherOptions(std::shared_ptr<IOOptions> io_opts) : io_options{io_opts} {}
    std::shared_ptr<IOOptions> io_options;
};

class Gatherer  {
public:
    Gatherer(const std::shared_ptr<GatherOptions> options) : options_{options} {}
    Gatherer(const std::shared_ptr<IOOptions> options) : options_{std::make_shared<GatherOptions>(options)} {}
    void init() {
        //nothing to do
    }
    void run() {
        gatherResults();
    }
    void done() {
        //nothing to do
    }
private:
    const std::shared_ptr<GatherOptions> options_;
    
    void gatherResults() {
        SubtaskResultsDB subtask_results_db; 
        TaskResultsDB task_results_db;
        const std::string& subtask_results_filename = options_->io_options->subtasksResultsFileName;
        const std::string& task_results_filename = options_->io_options->tasksResultsFileName;
        if(std::filesystem::exists(subtask_results_filename)) {
            boost::interprocess::file_lock flock(subtask_results_filename.c_str());
            subtask_results_db.readFromFile(subtask_results_filename);
        };
        if(std::filesystem::exists(task_results_filename)) {
            boost::interprocess::file_lock flock(task_results_filename.c_str());
            task_results_db.readFromFile(task_results_filename);
            gatherResults_(subtask_results_db, task_results_db);
            task_results_db.writeToFile(task_results_filename);
        } else {
            gatherResults_(subtask_results_db, task_results_db);
            task_results_db.writeToFile(task_results_filename);
        }
    }

    void gatherResults_(const SubtaskResultsDB& subtask_results_db, TaskResultsDB& task_results_db) {
        std::vector<std::vector<SubtaskResultsRecord>> finished_tasks = subtask_results_db.getFinished();
        for(auto subtasks : finished_tasks) {
            TaskRecord& task = static_cast<TaskRecord&>(subtasks[0]);
            std::vector<SubtaskResults> subtask_results;
            for(SubtaskResultsRecord& it: subtasks) {
                subtask_results.push_back(it.results);
            };
            Chi2BasedTest<Histogram> chi2_based_test; // no matter what histogram class is used here
            StatiscticalTestResults res = chi2_based_test.collect(task.problem, subtasks[0].subtask.Ntasks, subtask_results);
            TaskResultsRecord task_results_{task.meta, task.rng, task.repack, task.problem, res};
            task_results_db.push_back(task_results_);
        }
    }
};
