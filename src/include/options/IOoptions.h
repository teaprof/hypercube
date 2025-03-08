#ifndef __IO_OPTIONS_H__
#define __IO_OPTIONS_H__
#include <options/BasicOptions.h>

class IOOptions : public OptionsGroup {
    public:
    IOOptions(const std::string& io_options_name) : OptionsGroup(io_options_name) {
        namespace po = boost::program_options;
        addPositionalVisible("input", 1, po::value(&subtasksFileName)->default_value("subtasks.json"), "file with subtasks");
        addPositionalVisible("output", 1, po::value(&subtasksResultsFileName)->default_value("subtasks_results.json"), "file where subtask results should be written to");
        addPositionalVisible("gatherfile", 1, po::value(&tasksResultsFileName)->default_value("tasks_results.json"), "file where task results should be gathered to");
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

class GatherIOOptions : public OptionsGroup {
    public:
    GatherIOOptions() : OptionsGroup("gatherer io options") {
        namespace po = boost::program_options;
        addPositionalVisible("input", 1, po::value(&subtasksResultsFileName)->default_value("subtasks_results.json"), "file with subtasks");
        addPositionalVisible("output", 1, po::value(&tasksResultsFileName)->default_value("tasks_results.json"), "file where subtask results should be written to");
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

class GeneratorOutputOptions : public OptionsGroup {
    public:
    GeneratorOutputOptions() : OptionsGroup("subtasks output options") {
        namespace po = boost::program_options;
        addPositionalVisible("output", 1, po::value(&subtasksFileName)->default_value("subtasks.json"), "file with subtasks to generate");
    }
    void validate() override {
        if(subtasksFileName.empty()) {
            throw std::runtime_error("output file name should not be empty");
        }
    }
    std::string subtasksFileName;
};

#endif