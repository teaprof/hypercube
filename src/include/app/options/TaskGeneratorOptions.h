#ifndef __TASK_GENERATOR_OPTIONS_H__
#define  __TASK_GENERATOR_OPTIONS_H__

#include <ProgramOptionsHeavy.h>
#include <stattests/stat_tests/chi2based/StatisticalTestBase.h>

class RuntimeOptions : public program_options_heavy::OptionsGroup {
    public:
        RuntimeOptions() : OptionsGroup("Runtime options") {
            namespace po = boost::program_options;
            addPartialVisible("ram", po::value<size_t>(&max_ram_MB)->default_value(1024), "max memory available on target machine in MBytes (default is 1 GB)");
            addPartialVisible("maxTotalPoints", po::value<size_t>(&maxTotalPoints)->default_value(1e+12), "filter out tasks with the number of points greater than this value");
            addPartialVisible("maxSubtasks", po::value<size_t>(&maxSubtasks)->default_value(1e+3), "filter out tasks which have number of subtasks greater than this value");
            addPartialVisible("maxTasks", po::value<size_t>(&maxTasks)->default_value(1e+5), "stop when the number of generated tasks reachs this value");
        }        
        size_t max_ram_MB;
        size_t maxTotalPoints; // if the number of points required for specific test is greater than this value, the subtasks for this test will not be generated
        size_t maxSubtasks;
        size_t maxTasks;
};


#endif