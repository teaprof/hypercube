#ifndef __TASK_GENERATOR_OPTIONS_H__
#define  __TASK_GENERATOR_OPTIONS_H__

#include <options/BasicOptions.h>
#include <stat_tests/chi2based/StatisticalTestBase.h>

class RuntimeOptions : public OptionsGroupStorage {
    public:
        RuntimeOptions() : OptionsGroupStorage("Runtime options") {
            namespace po = boost::program_options;
            addPartialVisible("ram", po::value<size_t>(&RAM)->default_value(1), "max memory available on target machine");
        }        
        size_t RAM;
};


#endif