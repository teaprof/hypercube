#include <ProgramOptionsHeavy.h>
#include <cassert>
#include <iostream>
#include "app/singlerun.h"
#include "app/gatherer.h"
#include "app/taskgenerator.h"
#include <boost/interprocess/sync/file_lock.hpp>

#include<memory>

class MyProgramOptions : public program_options_heavy::SubcommandsParser {
    public:
    MyProgramOptions(int argc, const char* argv[]) : SubcommandsParser(argc, argv) {
        single_run_options = std::make_shared<SingleRunOptions>();
        gather_options = std::make_shared<GatherOptions>();
        task_generator_options = std::make_shared<TaskGeneratorOptions>();
        help_options = std::make_shared<program_options_heavy::HelpSubcommand>();
        program_description="Hypercube statistical test for random number generators.";
        push_back("generate", task_generator_options);
        push_back("run", single_run_options->manual_single_run_options); // run subtasks and gather
        push_back("subtask", single_run_options->batched_run_options); // run subtasks and gather
        push_back("gather", gather_options); // gather only
        push_back("default", help_options);
        this->setDefaultSubcommand("default", true);

        (*this)["run"]->program_description="run manually specified task";
        (*this)["subtask"]->program_description="run the specified subtasks from subtask json file";
        (*this)["gather"]->program_description="gather results of the finished subtasks and update final statistics";
        (*this)["generate"]->program_description="generate subtasks";
        //(*this)["default"]->program_description="--help - produce this help";
    }
    std::shared_ptr<SingleRunOptions> single_run_options;
    std::shared_ptr<GatherOptions> gather_options;
    std::shared_ptr<TaskGeneratorOptions> task_generator_options;
    std::shared_ptr<program_options_heavy::HelpSubcommand> help_options;
};

class MyApplication {
    public:
        MyApplication(int argc, const char* argv[]) : options_(argc, argv) {
            options_.parse(argc, argv);
        }
        MyProgramOptions& options() {
            return options_;
        }
        void init() {
            if(options_.selectedSubcommandName() == "run" || options_.selectedSubcommandName() == "subtask") {
                single_run.emplace(options_.single_run_options);
                gatherer.emplace(options_.single_run_options->io_options);
                single_run->init();
                gatherer->init();
            } else if(options_.selectedSubcommandName() == "gather") {
                gatherer.emplace(options_.gather_options);
                gatherer->init();
            } else if(options_.selectedSubcommandName() == "generate") {
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
        if(argc==1 || app.options().help_options->help_options->needHelp()) {
            program_options_heavy::printers::ProgramSubcommandsPrinter printer;
            auto dom = printer.print(app.options());

            program_options_heavy::printers::PrettyPrinter pp;
            dom->accept(pp);
    
            return 0;
        }
        app.init();
        app.    run();
        app.done();
        std::cout<<"Success"<<std::endl;
        return 0;
    } catch (std::runtime_error& r) {
        std::cerr<<"Error:"<<std::endl;
        std::cerr<<r.what()<<std::endl;
        return 1;
    }
}