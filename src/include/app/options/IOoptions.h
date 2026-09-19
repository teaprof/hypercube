#ifndef __IO_OPTIONS_H__
#define __IO_OPTIONS_H__
#include <ProgramOptionsHeavy.h>

class IOOptions : public program_options_heavy::HeavyOptionsGroup {
    public:
    IOOptions(const std::string& io_options_name) : HeavyOptionsGroup(io_options_name) {
        namespace po = boost::program_options;
        addPositional("input", 1, std::ref(subtasksFileName), "file with subtasks")->valueSemantics().setDefaultValue("subtasks.json");
        addPositional("output", 1, std::ref(subtasksResultsFileName), "file where subtask results should be written to")->valueSemantics().setDefaultValue("subtasks_results.json");
        addPositional("gatherfile", 1, std::ref(tasksResultsFileName), "file where task results should be gathered to")->valueSemantics().setDefaultValue("tasks_results.json");
    }
    void validate() override {
        if(subtasksFileName.empty()) {
            throw std::runtime_error("input file name should not be empty");
        }
        if(subtasksResultsFileName.empty()) {
            throw std::runtime_error("outputfile name should not be empty");
        }
        if(tasksResultsFileName.empty()) {
            throw std::runtime_error("gatherfile name should not be empty");
        }
    }
    std::string subtasksFileName;
    std::string subtasksResultsFileName;
    std::string tasksResultsFileName;
};

class GatherIOOptions : public program_options_heavy::HeavyOptionsGroup {
    public:
    GatherIOOptions() : HeavyOptionsGroup("gatherer io options") {
        namespace po = boost::program_options;
        addPositional("input", 1, std::ref(subtasksResultsFileName), "file with subtasks")->valueSemantics().setDefaultValue("subtasks_results.json");
        addPositional("output", 1, std::ref(tasksResultsFileName), "file where subtask results should be written to")->valueSemantics().setDefaultValue("tasks_results.json");
    }
    void validate() override {
        if(subtasksResultsFileName.empty()) {
            throw std::runtime_error("outputfile name should not be empty");
        }
        if(tasksResultsFileName.empty()) {
            throw std::runtime_error("gatherfile name should not be empty");
        }
    }
    std::string subtasksResultsFileName;
    std::string tasksResultsFileName;
};

class GeneratorOutputOptions : public program_options_heavy::HeavyOptionsGroup {
    public:
    GeneratorOutputOptions() : HeavyOptionsGroup("subtasks output options") {
        namespace po = boost::program_options;
        addPositional("output", 1, std::ref(subtasksFileName), "file with subtasks to generate")->valueSemantics().setDefaultValue("subtasks.json");
    }
    void validate() override {
        if(subtasksFileName.empty()) {
            throw std::runtime_error("output file name should not be empty");
        }
    }
    std::string subtasksFileName;
};

#endif