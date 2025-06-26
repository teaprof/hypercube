#ifndef __METADATA_OPTIONS_H__
#define __METADATA_OPTIONS_H__
#include <ProgramOptionsHeavy.h>
#include <boost/optional.hpp>

class MetaDataOptions : public program_options_heavy::OptionsGroup {
    public:
        MetaDataOptions() : OptionsGroup("Metadata options") {
            namespace po = boost::program_options;
            addPartialVisible("id", po::value(&taskId), "load task with taskId from input file");
            addPartialVisible("all,a", po::bool_switch(&runall), "run all files from the specified file");
            addPartialVisible("gather", po::bool_switch(&gather), "gather results [default]");
        }   
        void update(const boost::program_options::variables_map& vm) override {
            if(!taskId && !runall) {
                gather = true;
            }
        }
        void validate() override {
            size_t count = 0;
            if(taskId) {
                count++;
            }
            if(gather) {
                count++;
            }
            if(runall) {
                count++;
            }
            if(count != 1) {
                throw std::runtime_error("--id, --all or --gather should be specified");
            }
        }
        bool isRunMode() {
            return runall || taskId;
        }
        bool isGatherMode() {
            return gather;
        }
    boost::optional<uint64_t> taskId;
    bool gather;   
    bool runall;
};

#endif