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
#include "gatherer.h"
#include "taskgenerator.h"
#include <boost/interprocess/sync/file_lock.hpp>

#include<memory>

class MyProgramOptions : public ProgramModesOptions {
    public:
    MyProgramOptions() {
        program_description="Hypercube statistical test for random number generators.";        
        (*this)["all"].setTitle("Run all options");
        (*this)["all"].setHeader("run the default set of tests.");
        push_back("run", single_run_options);        
        push_back("gather", gather_options);
        push_back("generate", task_generator_options);
        this->defaultMode().addGroup(help_options);
    }
    SingleRunOptions single_run_options;
    GatherOptions gather_options;
    TaskGeneratorOptions task_generator_options;
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
            if(options_.selectedModeName() == "run") {
                
            }             
        }
        void run() {
        }
        void done() {
        }
    private:
        MyProgramOptions options_;
        std::optional<SingleRun> single_run;
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