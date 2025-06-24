#ifndef __SUB_TASK_OPTIONS_H__
#define  __SUB_TASK_OPTIONS_H__

#include <ProgramOptionsHeavy.h>
#include <stat_tests/chi2based/StatisticalTestBase.h>

class SubtaskOptions : public program_options_heavy::OptionsGroup {
    public:
        SubtaskOptions() : OptionsGroup("Subtask options") {
            namespace po = boost::program_options;
            addPartialVisible("nsubtasks", po::value<size_t>(&nSubtasks)->default_value(1), "total count of subtasks");
            addPartialVisible("subtask", po::value<size_t>(&curSubtask)->default_value(0), "number of the current subtask");
        }        
        SubtaskParameters subtask() {
            SubtaskParameters subtask{.Ntasks=nSubtasks, .cur_task=curSubtask};
            return subtask;
        }
        size_t nSubtasks{1};
        size_t curSubtask{0};
};


#endif