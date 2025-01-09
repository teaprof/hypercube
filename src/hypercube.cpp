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
            tasks_ = loadFromDB();
            if(tasks_.empty()) {
                tasks_.push_back(loadFromOptions());
            }
            for(auto it : tasks_)
                std::cout<<it;
        }

        void run() {
            size_t nThreads = options_.multithread_options.nThreads();

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
                results_.push_back(cur_res);
                times_.push_back(time);
            }
        }
        void done() {
            if(options_.io_options.outputFileName) {
                const std::string& filename = *(options_.io_options.outputFileName);
                saveResults(filename);
            }
        }
    private:
        MyProgramOptions options_;
        std::vector<TaskRecord> tasks_;
        std::vector<SubtaskResults> results_;
        std::vector<double> times_;

        std::vector<TaskRecord> loadFromDB() {
            std::vector<TaskRecord> res;
            if(options_.meta_data_options.taskId || options_.meta_data_options.runall) {
                TaskDB taskDB;
                assert(options_.io_options.inputFileName);
                taskDB.readFromFile(*options_.io_options.inputFileName);
                if(options_.meta_data_options.taskId) {
                    auto task = taskDB.find(*options_.meta_data_options.taskId);
                    if(!task) {
                        std::stringstream str;
                        str<<"Input file "<<*options_.io_options.inputFileName<<" doesn't contain task with taskid = "<<*options_.meta_data_options.taskId;
                        throw std::runtime_error(str.str());
                    }
                    res.push_back(*task);
                } else {
                    assert(options_.meta_data_options.runall);
                    res = taskDB.records();
                    if(res.empty()) {
                        std::cout<<"The specified file doesn't contain any task"<<std::endl;
                    }
                }
            }
            return res;
        }
        TaskRecord loadFromOptions() {
            TaskRecord res;
            res.problem = options_.hypercube_options.problem();
            res.subtask = SubtaskParameters{.Ntasks=options_.subtask_options.nSubtasks,.cur_task=options_.subtask_options.curSubtask};
            res.repack = options_.bits_repack_options.description();
            res.rng = options_.rng_options.description();        
            return res;
        }
        void saveResults(const std::string& filename) {
            TestResultsDB results_db; 
            if(std::filesystem::exists(filename)) {
                boost::interprocess::file_lock flock(filename.c_str());
                results_db.readFromFile(filename);
                results_db.add(tasks_, results_, times_);
                results_db.writeToFile(filename);
            } else {
                results_db.add(tasks_, results_, times_);
                results_db.writeToFile(filename);
            }
        }
};


int main(int argc, char* argv[]) {
    try {                
        MyApplication app(argc, argv);
        app.init();
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