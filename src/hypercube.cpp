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

class DefaultOptions : public ProgramOptions {
    public:
    DefaultOptions() {
        addGroup(help_options);
    }
    HelpOptions help_options;
};

class MyProgramOptions : public ProgramModesOptions {
    public:
    MyProgramOptions() {
        program_description="Hypercube statistical test for random number generators.";        
        push_back("default", default_options);
        push_back("generate", task_generator_options);
        push_back("run", single_run_options.manual_single_run_options); // run subtasks and gather
        push_back("subtask", single_run_options.subtask_run_options); // run subtasks and gather
        push_back("gather", gather_options); // gather only
        this->setDefaultModeName("default");
        this->showDefaultModeName(false);

        //(*this)["all"].setTitle("Run all options");
        //(*this)["all"].setHeader("run the default set of tests.");
        (*this)["default"].setTitle("Help options");
        (*this)["run"].setTitle("Manual run options");
        (*this)["run"].setHeader("run manually specified task");
        (*this)["subtask"].setTitle("Run options");
        (*this)["subtask"].setHeader("run the specified subtasks from subtask json file");
        (*this)["gather"].setTitle("Gather only options");
        (*this)["gather"].setHeader("gather results of the finished subtasks and update final statistics");
        (*this)["generate"].setTitle("Generate subtasks options");
        (*this)["generate"].setHeader("generate subtasks.");
    }
    SingleRunOptions single_run_options;
    GatherOptions gather_options;
    TaskGeneratorOptions task_generator_options;
    DefaultOptions default_options;
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
                single_run.emplace(options_.single_run_options);
                gatherer.emplace(options_.single_run_options.io_options);
                single_run->init();
                gatherer->init();
            } else if(options_.selectedModeName() == "gather") {
                gatherer.emplace(options_.gather_options);
                gatherer->init();
            } else if(options_.selectedModeName() == "generate") {
                task_generator.emplace(options_.task_generator_options);
                task_generator->init();
            }
        }
        void run() {
            if(task_generator.has_value()) {
                task_generator->run();
            }
            if(single_run.has_value()) {
                single_run->run();
            }
            if(gatherer.has_value()) {
                gatherer->run();
            }
        }
        void done() {
            if(task_generator.has_value()) {
                task_generator->done();
            }
            if(single_run.has_value()) {
                single_run->done();
            }
            if(gatherer.has_value()) {
                gatherer->done();
            }
        }
    private:
        MyProgramOptions options_;
        std::optional<SingleRun> single_run;
        std::optional<Gatherer> gatherer;
        std::optional<TaskGenerator> task_generator;
};

int main(int argc, const char* argv[]) {
    try {                
        /*std::cout<<"Program have been run with the following options:"<<std::endl;
        for(int n = 0; n < argc; n++) {
            std::cout<<"    "<<argv[n]<<std::endl;
        }*/
        MyApplication app(argc, argv);
        if(argc==1 || app.options().default_options.help_options.needHelp()) {
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