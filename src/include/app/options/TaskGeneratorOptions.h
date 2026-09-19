#ifndef __TASK_GENERATOR_OPTIONS_H__
#define  __TASK_GENERATOR_OPTIONS_H__

#include <ProgramOptionsHeavy.h>
#include <stattests/stat_tests/chi2based/StatisticalTestBase.h>

class RuntimeOptions : public program_options_heavy::HeavyOptionsGroup {
    public:
        RuntimeOptions() : HeavyOptionsGroup("Runtime options") {
            namespace po = boost::program_options;
            addPartial("ram", std::ref(max_ram_MB), "max memory available on target machine in MBytes (default is 1 GB)")->valueSemantics().setDefaultValue(1024);
            addPartial("maxTotalPoints", std::ref(maxTotalPoints), "filter out tasks with the number of points greater than this value")->valueSemantics().setDefaultValue(1e+12);
            addPartial("maxSubtasks", std::ref(maxSubtasks), "filter out tasks which have number of subtasks greater than this value")->valueSemantics().setDefaultValue(1e+3);
            addPartial("maxTasks", std::ref(maxTasks), "stop when the number of generated tasks reachs this value")->valueSemantics().setDefaultValue(1e+5);
        }        
        size_t max_ram_MB;
        size_t maxTotalPoints; // if the number of points required for specific test is greater than this value, the subtasks for this test will not be generated
        size_t maxSubtasks;
        size_t maxTasks;
};


#endif