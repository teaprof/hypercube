#ifndef __IO_OPTIONS_H__
#define __IO_OPTIONS_H__
#include <options/BasicOptions.h>
#include <optional>

/// UNUSED NOW
class IOOptions : public PartialOptions {
    public:
    IOOptions() : PartialOptions("IO options") {
        namespace po = boost::program_options;
        addPositionalVisible("input", 1, po::value(&inputFileName), "input file name");
        addPositionalVisible("output", 1, po::value(&outputFileName), "output file name");
        //addPartialVisible("byid", po::value(&runFromCommandLine), "run task with the specified id");
        //addPartialVisible("sample", po::value(&runTaskById), "sampling mode");
        //addPartialVisible("gather", po::value(&gatherResultsByTaskId), "gathering mode");
    }
    boost::optional<std::string> inputFileName;
    boost::optional<std::string> outputFileName;
};

#endif