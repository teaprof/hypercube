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
#include "singlerun.h"
#include <boost/interprocess/sync/file_lock.hpp>

#include<memory>

class MyProgramOptions : public ProgramModesOptions {
    public:
    MyProgramOptions() {
        program_description="Hypercube statistical test for random number generators.";        
        (*this)["all"]->setTitle("Run all options");
        (*this)["all"]->setHead("run the default set of tests.");
        /*(*this)["run"].setTitle("Run options");
        (*this)["run"].addGroup(meta_data_options);
        (*this)["run"].addGroup(rng_options);
        (*this)["run"].addGroup(bits_repack_options);
        //(*this)["run"].addGroup(sampling_options);
        (*this)["run"].addGroup(hypercube_options);
        (*this)["run"].addGroup(subtask_options);
        (*this)["run"].addGroup(multithread_options);        
        (*this)["run"].addGroup(io_options);
        (*this)["run"].addGroup(help_options);*/
        single_run_options = std::make_shared<SingleRunOptions>();
        push_back("run", single_run_options);
        (*this)["gather"]->addGroup(io_options);
        (*this)["gather"]->setTitle("Gather options");
        (*this)["generate"]->addGroup(subtasks_output_options);
        (*this)["generate"]->setTitle("Generate options");
        this->defaultMode()->addGroup(help_options);
    }
    std::shared_ptr<SingleRunOptions> single_run_options;
    MetaDataOptions meta_data_options;
    RNGOptions rng_options;
    BitsRepackOptions bits_repack_options;
    //SamplingOptions sampling_options;
    HypercubeOptions hypercube_options;
    SubtaskOptions subtask_options;
    MultithreadOptions multithread_options;
    IOOptions io_options;
    SubtasksOutput subtasks_output_options;
    HelpOptions help_options;
};

class MyApplication {
    public:
        MyApplication(int argc, const char* argv[]) {
            options_.parse(argc, argv);
        }
        MyProgramOptions& options() {
            return options_;
        }
        void init() {
            assert(options_.selected_mode != options_.modes.end());
            if(options_.selected_mode->second->mode_name_ == "run") {
                single_run = std::make_shared<SingleRun>();
            }             
        }
        void run() {
        }
        void done() {
        }
    private:
        MyProgramOptions options_;
        std::shared_ptr<SingleRun> single_run;
};

int main(int argc, const char* argv[]) {
    try {                
        /*std::cout<<"Program have been run with the following options:"<<std::endl;
        for(int n = 0; n < argc; n++) {
            std::cout<<"    "<<argv[n]<<std::endl;
        }*/
        MyApplication app(argc, argv);
        if(app.options().help_options.needHelp()) {
            app.options().help().print(0);
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