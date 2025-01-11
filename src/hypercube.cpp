#include "stat_tests/chi2based/StatisticalTestBase.h"
#include "progress/tictoc.h"
#include "stat_tests/hypercube/HypercubeTest.h"
#include "TestResultsDB/SubtasksResultsDB.h"
#include "TestResultsDB/SubtaskDB.h"
#include "TestResultsDB/TaskResultsDB.h"
#include "options/RngOptions.h"
#include "options/SubtaskOptions.h"
#include "options/HypercubeOptions.h"
#include "options/ModeOptions.h"
#include "options/IOoptions.h"
#include "serializers/prettyserializer.h"
#include<cassert>
#include<iostream>
#include<filesystem>
#include "options/BasicOptions.h"
#include <boost/interprocess/sync/file_lock.hpp>

#include<memory>

class MyProgramOptions : public ProgramOptions {
    public:
    MyProgramOptions() {
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

class MyApplication {
    public:
        MyApplication(int argc, char* argv[]) {
            options_.parse(argc, argv);
        }
        ~MyApplication() {}

        const MyProgramOptions& options() const {
            return options_;
        }

        void init() {
            if(options_.meta_data_options.isRunMode()) {
                initSubtasks();
            }
        }

        void initSubtasks() {
            tasks_ = loadFromDB();
            if(tasks_.empty()) {
                tasks_.push_back(loadFromOptions());
            }            
            for(auto it : tasks_) {                
                std::cout<<it<<std::endl;
            }
        }

        void run() {
            if(options_.meta_data_options.isRunMode()) {
                runSubtask();
            }
        }
        void done() {
            //if(options_.io_options.subtasksResultsFileName) {
            //    const std::string& filename = *(options_.io_options.subtasksResultsFileName);
            //    saveResults(filename);
            //}
            saveResults(options_.io_options.subtasksResultsFileName);
            gatherResults();
        }
    private:
        MyProgramOptions options_;
        std::vector<SubtaskRecord> tasks_;
        std::vector<SubtaskResults> subtask_results_;
        std::vector<TaskResultsRecord> task_results_;
        std::vector<double> times_;

        void runSubtask() {
            size_t nThreads = options_.multithread_options.nThreads();
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

        void gatherResults() {
            SubtaskResultsDB subtask_results_db; 
            TaskResultsDB task_results_db;
            const std::string& subtask_results_filename = options_.io_options.subtasksResultsFileName;
            const std::string& task_results_filename = options_.io_options.tasksResultsFileName;
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

        std::vector<SubtaskRecord> loadFromDB() {
            std::vector<SubtaskRecord> res;
            if(options_.meta_data_options.taskId || options_.meta_data_options.runall) {
                SubtaskDB subtaskDB;
                subtaskDB.readFromFile(options_.io_options.subtasksFileName);
                if(options_.meta_data_options.taskId) {
                    auto task = subtaskDB.find(*options_.meta_data_options.taskId);
                    if(!task) {
                        std::stringstream str;
                        str<<"Input file "<<options_.io_options.subtasksFileName<<" doesn't contain task with taskid = "<<*options_.meta_data_options.taskId;
                        throw std::runtime_error(str.str());
                    }
                    res.push_back(*task);
                } else {
                    assert(options_.meta_data_options.runall);
                    res = subtaskDB.records();
                    if(res.empty()) {
                        std::cout<<"The specified file doesn't contain any task"<<std::endl;
                    }
                }
            }
            return res;
        }
        SubtaskRecord loadFromOptions() {
            SubtaskRecord res;
            res.problem = options_.hypercube_options.problem();
            res.subtask = SubtaskParameters{.Ntasks=options_.subtask_options.nSubtasks,.cur_task=options_.subtask_options.curSubtask};
            res.repack = options_.bits_repack_options.description();
            res.rng = options_.rng_options.description();        
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


int main(int argc, char* argv[]) {
    try {                
        std::cout<<"Program have been run with the following options:"<<std::endl;
        for(int n = 0; n < argc; n++) {
            std::cout<<"    "<<argv[n]<<std::endl;
        }
        MyApplication app(argc, argv);
        if(app.options().basic_options.needHelp()) {
            app.options().help();
            return 0;
        }
        app.init();
        app.run();
        app.done();
        return 0;
    } catch (std::runtime_error& r) {
        std::cerr<<"Error:"<<std::endl;
        std::cerr<<r.what()<<std::endl;
        return 1;
    }
}