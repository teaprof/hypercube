#ifndef __RNG_ARCHIVE_OPTIONS_H__
#define __RNG_ARCHIVE_OPTIONS_H__

#include <ProgramOptionsHeavy.h>
#include <stattests/stat_tests/rng.h>
#include <functional>

class RNGArchiveOptions : public program_options_heavy::HeavyOptionsGroup {
    public:

        RNGArchiveOptions() : HeavyOptionsGroup("RNG archive options") {
            namespace po = boost::program_options;
            addPartial("rngarchive", std::ref(filename), "rng archive name")->valueSemantics().setDefaultValue("rng.bin");
        }            
        void validate() override {
        }
        void update(const boost::program_options::variables_map& map) override {
            //nothing to do
        }
        std::optional<RNGArchiveDescription> description() {
            return RNGArchiveDescription{filename};
        }
        std::string filename; // rng name        
};


#endif
