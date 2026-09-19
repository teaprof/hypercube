#ifndef __SUB_TASK_OPTIONS_H__
#define  __SUB_TASK_OPTIONS_H__

#include <ProgramOptionsHeavy.h>
#include <stattests/stat_tests/chi2based/StatisticalTestBase.h>

class SubtaskOptions : public program_options_heavy::HeavyOptionsGroup {
    public:
        SubtaskOptions() : HeavyOptionsGroup("Subtask options") {
            namespace po = boost::program_options;            
            addPartial("nsubtasks", std::ref(nSubtasks), "total count of subtasks")->valueSemantics().setDefaultValue(1);
            addPartial("subtask", std::ref(curSubtask), "number of the current subtask")->valueSemantics().setDefaultValue(0);
        }        
        SubtaskParameters subtask() {
            SubtaskParameters subtask{.Ntasks=nSubtasks, .cur_task=curSubtask};
            return subtask;
        }
        size_t nSubtasks{1};
        size_t curSubtask{0};
};


#endif