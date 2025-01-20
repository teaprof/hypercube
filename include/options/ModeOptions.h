#ifndef __MODE_OPTIONS_H__
#define __MODE_OPTIONS_H__

#include <map>

#include "options/BasicOptions.h"

class Mode : public ProgramOptions {
    public:
        //Mode(const std::string_view& modename) : modename_(modename) { }
        
        void help() {
            std::cout<<"Mode "<<modename_<<" options "<<std::endl;
            std::cout<<visible<<std::endl;
        }
        std::string modename_;
};

class ProgramModesOptions {
    public:
    Mode& operator[](const std::string& str) {
        if(!modes.contains(str)) {
            modes.emplace(str, std::move(Mode{}));
        }
        return modes.at(str);
    }
    void setDefaultMode(const std::string& mode) {
        default_mode_ = mode;
    }
    bool parse(int argc, const char* argv[]) {
        assert(!modes.empty());
        isDefault = false;
        if(argc < 2) {
            isDefault = true;
            selected_mode = modes.find(default_mode_); //could be modes.end()
            return false;
        };
        const char* first_arg = argv[1];
        selected_mode = modes.find(first_arg);
        if(selected_mode == modes.end()) {
            std::stringstream str;
            str<<"unknown mode: "<<first_arg;
            throw std::runtime_error(str.str());
        };
        argc--;
        argv++;
        selected_mode->second.parse(argc, argv);
        return true;
    }
    std::map<std::string, Mode>::iterator selected_mode;
    std::map<std::string, Mode> modes;
    std::string default_mode_{"default"};
    bool isDefault{false};
};

void ttt() {
    ProgramModesOptions modes;
    PartialOptions runOptions("run group");
    PartialOptions gatherOptions("gather group");
    PartialOptions commonOptions("common group");
    modes["run"].addGroup(runOptions);
    modes["run"].addGroup(commonOptions);
    modes["gather"].addGroup(gatherOptions);
    modes["gather"].addGroup(commonOptions);
}

#endif