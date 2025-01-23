#ifndef __MODE_OPTIONS_H__
#define __MODE_OPTIONS_H__

#include <map>
#include <optional>

#include "options/BasicOptions.h"

class Mode : public ProgramOptions {
    public:
        Mode(const std::string& mode_name) : mode_name_(mode_name) {}
        void setHelp(const std::string& str) {
            helpstr = str;
        }
        void setTitle(const std::string& str) {
            title_ = str;
        }
        
        TextSection help() const {
            TextSection section;
            section.title << title_;
            std::stringstream str;            
            section.body<<visible;
            return section;
        }
        std::string mode_name_;
        std::string helpstr;
        std::string title_{""};
};

class ProgramModesOptions {
    public:
    ProgramModesOptions() {
        (*this)[default_mode_].setTitle("");
    }
    Mode& operator[](const std::string& mode_name) {
        auto pos = find(mode_name);
        if(pos == modes.end()) {
            modes.emplace_back(mode_name);
            return modes.back();
        }
        return *pos;
    }
    std::vector<Mode>::iterator find(const std::string& mode_name) {
        return std::find_if(modes.begin(), modes.end(), [&mode_name](auto& it) { return it.mode_name_ == mode_name;});
    }
    std::vector<Mode>::const_iterator find(const std::string& mode_name) const {
        return std::find_if(modes.begin(), modes.end(), [&mode_name](auto& it) { return it.mode_name_ == mode_name;});
    }

    Mode& defaultMode() {
        return (*this)[default_mode_];
    }
    void setDefaultMode(const std::string& mode) {
        default_mode_ = mode;
    }
    bool parse(int argc, const char* argv[]) {
        assert(!modes.empty());
        if(argc > 2) {
            const char* first_arg = argv[1];
            selected_mode = find(first_arg);
            if(selected_mode != modes.end()) {
                argc--;
                argv++;
            } else {
                // Case: mode is unknown, fallback to default mode
                selected_mode = find(default_mode_);
                assert(selected_mode != modes.end());
            }
        } else {
            // Case: no options specified, fallback to default mode
            selected_mode = find(default_mode_);
            assert(selected_mode != modes.end());
        };
        selected_mode->parse(argc, argv);
        return true;
    }
    TextSection help() const {
        TextSection section;
        section.header<<"Usage:\n";
        section.header<<"\t"<<exename<<" --help\n";
        for(auto& mode : modes) {
            if(mode.mode_name_ != default_mode_) {
                section.header<<"\t"<<exename<<" "<<mode.mode_name_<<" ["<<mode.mode_name_<<" options] ...\n";
            }
        }
        TextSection description;
        description.title<<"Detailed description:";
        description.header<<program_description;;
        section.subsections.push_back(std::move(description));
        TextSection options;
        options.title<<"Options:";
        options.subsections.push_back(find(default_mode_)->help());
        for(auto& mode : modes) {
            if(mode.mode_name_ != default_mode_) {
                options.subsections.push_back(mode.help());
            }
        }
        section.subsections.push_back(std::move(options));
        return section;
    }
    std::string program_description;
    std::string exename{"hypercube "};
    std::vector<Mode>::iterator selected_mode;
    std::vector<Mode> modes;
    std::string default_mode_{"default"};
};

#endif