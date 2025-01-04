#ifndef __MODE_OPTIONS_H__
#define __MODE_OPTIONS_H__
#include <options/BasicOptions.h>
#include <boost/optional.hpp>

/// UNUSED NOW
class ModeOptions : public PartialOptions {
    public:
    ModeOptions() : PartialOptions("Mode options") {
        namespace po = boost::program_options;
        //addPartialVisible("byid", po::value(&runFromCommandLine), "run task with the specified id");
        //addPartialVisible("sample", po::value(&runTaskById), "sampling mode");
        //addPartialVisible("gather", po::value(&gatherResultsByTaskId), "gathering mode");
    }
    boost::optional<bool> runFromCommandLine;
    boost::optional<bool> runTaskById;
    boost::optional<bool> gatherResultsByTaskId;
};

class MetaDataOptions : public PartialOptions {
    public:
        MetaDataOptions() : PartialOptions("Metadata options") {
            namespace po = boost::program_options;
            addPartialVisible("id", po::value(&taskId), "load task with taskId from input file");
        }        
    boost::optional<uint64_t> taskId;
};

#endif