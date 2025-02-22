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
#include <cassert>
#include <iostream>
#include <filesystem>
#include "options/BasicOptions.h"
#include <boost/interprocess/sync/file_lock.hpp>

#include<memory>


class ManualRunOptions : public ProgramOptions {
public:
    ManualRunOptions() : ProgramOptions() {
        rng_options = std::make_shared<RNGOptions>();
        bits_repack_options = std::make_shared<BitsRepackOptions>();
        //sampling_options = std::make_shared<SamplingOptions>();
        hypercube_options = std::make_shared<HypercubeOptions>();
        subtask_options = std::make_shared<SubtaskOptions>();
        addGroup(rng_options);
        addGroup(bits_repack_options);
        //addGroup(sampling_options);
        addGroup(hypercube_options);
        addGroup(subtask_options);
    }
    std::shared_ptr<RNGOptions> rng_options;
    std::shared_ptr<BitsRepackOptions> bits_repack_options;
    //std::shared_ptr<SamplingOptions> sampling_options;
    std::shared_ptr<HypercubeOptions> hypercube_options;
    std::shared_ptr<SubtaskOptions> subtask_options;
};

class BatchedRunOptions : public ProgramOptions {
public:
    BatchedRunOptions() : ProgramOptions() {
        meta_data_options = std::make_shared<MetaDataOptions>();
        addGroup(meta_data_options);
    }
    std::shared_ptr<MetaDataOptions> meta_data_options;
};


class SingleRunOptions  { // maybe rename to RunOptions
public:
    SingleRunOptions() {
        manual_single_run_options = std::make_shared<ManualRunOptions>();
        batched_run_options = std::make_shared<BatchedRunOptions>();
        multithread_options = std::make_shared<MultithreadOptions>();
        io_options = std::make_shared<IOOptions>();
        manual_single_run_options->addGroup(multithread_options);
        manual_single_run_options->addGroup(io_options);
        batched_run_options->addGroup(multithread_options);
        batched_run_options->addGroup(io_options);
    }
    std::shared_ptr<ManualRunOptions> manual_single_run_options;
    std::shared_ptr<BatchedRunOptions> batched_run_options;

    std::shared_ptr<MultithreadOptions> multithread_options;
    std::shared_ptr<IOOptions> io_options;
};

class SingleRun {   // maybe rename to Runner     
    public:
        SingleRun(const std::shared_ptr<SingleRunOptions>& options) : options_{options} {
        }
        ~SingleRun() {}

        const SingleRunOptions& options() const {
            return *options_;
        }


        void init() {
            initSubtasks();
        }

        void initSubtasks() {
            if(options_->batched_run_options->activated) {
                assert(!options_->manual_single_run_options->activated);
                tasks_ = loadFromDB();
            } else {
                assert(options_->manual_single_run_options->activated);
                tasks_ = loadFromOptions();
            };
            std::cout<<"The following tasks have been created:\n";
            for(auto it : tasks_) {                
                std::cout<<it<<std::endl;
            }
        }

        void run() {
            runSubtasks();
        }
        void done() {
            saveResults(options_->io_options->subtasksResultsFileName);
        }
    private:
        std::shared_ptr<SingleRunOptions> options_;
        std::vector<SubtaskRecord> tasks_;
        std::vector<SubtaskResults> subtask_results_;
        std::vector<TaskResultsRecord> task_results_;
        std::vector<double> times_;

        void runSubtasks() {
            size_t nThreads = options_->multithread_options->nThreads();
            nThreads = 1;

            for(auto task : tasks_) {
                auto rng = createGenerator(task.rng, task.repack); 
                auto sampler = std::make_shared<HypercubeSampler>(task.problem);
                Chi2BasedTest<HistogramAtomic> test;
                
                tic();
                auto cur_res = test.run(task.problem, task.subtask, sampler, rng->copy(), nThreads);
                double time = toc();        
                tic();
                std::cout<<"sum = "<<cur_res.sum<<", sum2 = "<<cur_res.sum2<<std::endl;
                std::cout<<"elapsed: "<<time*1000<<" ms"<<std::endl;
                subtask_results_.push_back(cur_res);
                times_.push_back(time);
            }
        }

        std::vector<SubtaskRecord> loadFromDB() {
            std::vector<SubtaskRecord> res;
            assert(options_->batched_run_options->activated);
            auto meta_data_options = options_->batched_run_options->meta_data_options;
            if(meta_data_options->taskId || meta_data_options->runall) {
                SubtaskDB subtaskDB;
                subtaskDB.readFromFile(options_->io_options->subtasksFileName);
                if(meta_data_options->taskId) {
                    auto task = subtaskDB.find(*meta_data_options->taskId);
                    if(!task) {
                        std::stringstream str;
                        str<<"Input file "<<options_->io_options->subtasksFileName<<" doesn't contain task with taskid = "<<*meta_data_options->taskId;
                        throw std::runtime_error(str.str());
                    }
                    res.push_back(*task);
                } else {
                    assert(meta_data_options->runall);
                    res = subtaskDB.records();
                    if(res.empty()) {
                        std::cout<<"The specified file doesn't contain any task"<<std::endl;
                    }
                }
            }
            return res;
        }
        std::vector<SubtaskRecord> loadFromOptions() {
            std::vector<SubtaskRecord> res;
            assert(options_->manual_single_run_options->activated);            
            for(auto problem : *options_->manual_single_run_options->hypercube_options) {
                for(auto bits_repack_description : *options_->manual_single_run_options->bits_repack_options) {
                    SubtaskRecord subtask;
                    subtask.problem = problem;
                    subtask.subtask = SubtaskParameters{.Ntasks=options_->manual_single_run_options->subtask_options->nSubtasks,.cur_task=options_->manual_single_run_options->subtask_options->curSubtask};
                    subtask.repack = bits_repack_description;
                    subtask.rng = options_->manual_single_run_options->rng_options->description();
                    res.push_back(std::move(subtask));
                }
            }
            return res;
        }
        void saveResults(const std::string& filename) {
            SubtaskResultsDB results_db; 
            if(std::filesystem::exists(filename)) {
                boost::interprocess::file_lock flock(filename.c_str());
                results_db.readFromFile(filename);
                results_db.add(tasks_, subtask_results_, times_);
                results_db.writeToFile(filename);
            } else {
                results_db.add(tasks_, subtask_results_, times_);
                results_db.writeToFile(filename);
            }
        }
};
