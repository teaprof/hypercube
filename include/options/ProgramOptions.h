#ifndef __MODE_OPTIONS_H__
#define __MODE_OPTIONS_H__

#include <options/prettyprinter.h>

#include <map>
#include <string> 
#include <sstream>
#include <iostream>
#include <variant>

#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>

class DocumentedOptionsGroup {
    public:
        void setTitle(const std::string& str) {
            title_ = str;
        }
        void setHeader(const std::string& str) {
            header_ = str;
        }        
        void setFooter(const std::string& str) {
            footer_ = str;
        }        
    protected:
        //These values are used to generate documentation
        std::string title_;
        std::string header_;
        std::string footer_;
};

class OptionsGroupStorage : public DocumentedOptionsGroup {
    public:
        static constexpr uint16_t text_width = 140;
        OptionsGroupStorage(std::string caption) : visible(caption, text_width) {}

        template<class...Args>
        auto addPartialVisible(Args ... args) {
            //this option will be used in parsing and will be printed in the help message
            auto option = boost::make_shared<boost::program_options::option_description>(args...);            
            partial.add(option);
            visible.add(option);
            return option;
        }
        void addPositionalHidden(std::string name, int count, const boost::program_options::value_semantic* s) {
            //this option will be used in parsing, but will not be printed in the help message
            positional.add(name.c_str(), count);
            partial.add_options()(name.c_str(), s);
        }
        auto addPositionalVisible(std::string name, int count, const boost::program_options::value_semantic* s, std::string description) {
            //this option will be used in parsing and will be printed in the help message
            auto option = boost::make_shared<boost::program_options::option_description>(name.c_str(), s, description.c_str());
            positional.add(name.c_str(), count);
            partial.add(option);
            visible.add(option);
            return option;
        }

        virtual void update(const boost::program_options::variables_map& vm) {
            //nothing to do            
        }
        virtual void validate() {
            //nothing to do
        }

        TextSection help() const {
            TextSection section;
            section.title << title_;
            section.header << header_;
            section.body << visible;
            section.footer << footer_;
            return section;
        }

        //These vars are used only for parsing arguments
        boost::program_options::options_description partial;
        boost::program_options::positional_options_description positional;        

        //These vars are used only for printing help message
        boost::program_options::options_description visible;
        boost::program_options::options_description hidden;         
};

class ProgramOptions : public DocumentedOptionsGroup {
    // This class can parse the list of options and print the help message
    // Use this class for simple set of command line options like: 
    // programname --arg1 --arg2 10 -zxc -v 20 input.txt output.txt
    public:
        ProgramOptions() {}
        virtual void addGroup(OptionsGroupStorage& options) {
            options_.push_back(options);
            if(options.positional.max_total_count() != 0) {
                for(auto it : options_) {
                    //only one group of options is allowed to have positional arguments
                    assert(it.get().positional.max_total_count() == 0);
                }
            }
        }
        virtual void parse(int argc, const char* argv[]) {
            namespace po = boost::program_options;
            boost::program_options::options_description partial;
            boost::program_options::positional_options_description positional;        
            for(auto it : options_) {
                partial.add(it.get().partial);
                if(it.get().positional.max_total_count() != 0) {
                    //only one group of options is allowed to have positional arguments
                    assert(positional.max_total_count() == 0);
                    positional = it.get().positional;
                }
            }
            boost::program_options::variables_map vm;
            po::store(po::command_line_parser(argc, argv).options(partial).positional(positional).run(), vm);
            boost::program_options::notify(vm);
            for(auto it : options_) {
                it.get().update(vm);
            }
        }
        void validate() {
            for(auto it : options_) 
                it.get().validate();
        }
        TextSection help() const {
            TextSection section;
            section.title << title_;
            section.header << header_;
            for(auto it : options_) {
                section.subsections.push_back(it.get().help());
            }
            section.footer << footer_;
            return section;
        }
    private:
        std::vector<std::reference_wrapper<OptionsGroupStorage>> options_;
    };


class ProgramModesOptions : public DocumentedOptionsGroup {
    using value_t = std::variant<std::reference_wrapper<ProgramOptions>, std::shared_ptr<ProgramOptions>>;
    using modes_t = std::map<std::string, value_t>;
    public:
    ProgramModesOptions() {}    
    ProgramOptions& push_back(const std::string& mode_name, ProgramOptions& val) {
        auto res = modes.emplace(mode_name, std::ref(val));
        if(!res.second) {
            throw std::runtime_error("The specified mode_name is already present in ProgramModesOptions");
        }
        modes_order.push_back(modes.find(mode_name));
        return getref(res.first);
    }
    ProgramOptions& at(const std::string& mode_name) {
        auto pos = modes.find(mode_name);
        if(pos != modes.end()) {
            throw std::out_of_range("the requested mode_name is not contained in the mode list");
        }
        return getref(pos);
    }    
    ProgramOptions& operator[](const std::string& mode_name) {
        auto pos = modes.find(mode_name);
        if(pos == modes.end()) {
            pos = modes.emplace(mode_name, std::make_shared<ProgramOptions>()).first;
        }
        return getref(pos);
    }    
    ProgramOptions& defaultMode() {
        return (*this)[default_mode_name_];
    }
    void setDefaultMode(const std::string& mode) {        
        default_mode_name_ = mode;
    }
    ProgramOptions& selectedMode() {
        return getref(selected_mode_);
    }
    const std::string& selectedModeName() {
        return selected_mode_->first;
    }
    bool parse(int argc, const char* argv[]) {
        assert(!modes.empty());
        if(argc > 2) {
            const char* first_arg = argv[1];
            //selected_mode_ = find(first_arg);
            if(selected_mode_ != modes.end()) {
                argc--;
                argv++;
            } else {
                // Case: mode is unknown, fallback to default mode
                selected_mode_ = modes.find(default_mode_name_);
                assert(selected_mode_ != modes.end());
            }
        } else {
            // Case: no options specified, fallback to default mode
            selected_mode_ = modes.find(default_mode_name_);
            assert(selected_mode_ != modes.end());
        };
        getref(selected_mode_).parse(argc, argv);
        return true;
    }
    TextSection help() const {
        TextSection section;
        section.header<<header_;
        section.header<<"Usage:\n";
        section.header<<"\t"<<exename<<" --help\n";
        for(auto& mode : modes_order) {            
            if(mode->first != default_mode_name_) {
                section.header<<"\t"<<exename<<" "<<mode->first<<" ["<<mode->first<<" options] ...\n";
            }
        }
        TextSection description;
        description.title<<"Detailed description:";
        description.header<<program_description;;
        section.subsections.push_back(std::move(description));
        TextSection options;
        options.title<<"Options:";
        options.subsections.push_back(getref(modes.find(default_mode_name_)).help());
        for(auto& mode : modes_order) {
            if(mode->first != default_mode_name_) {
                options.subsections.push_back(getref(mode).help());
            }
        }
        section.subsections.push_back(std::move(options));
        section.footer<<footer_;
        return section;
    }
    std::string program_description;
    std::string exename{"hypercube "};
private:
    modes_t modes;    
    std::vector<modes_t::iterator> modes_order; //for printing purpose
    modes_t::iterator selected_mode_;
    std::string default_mode_name_{"default"};
    ProgramOptions& getref(modes_t::iterator it) {
        if(std::holds_alternative<std::reference_wrapper<ProgramOptions>>(it->second)) {
            return std::get<std::reference_wrapper<ProgramOptions>>(it->second);
        } else {
            return *std::get<std::shared_ptr<ProgramOptions>>(it->second);
        }
    }
    const ProgramOptions& getref(modes_t::const_iterator it) const {
        if(std::holds_alternative<std::reference_wrapper<ProgramOptions>>(it->second)) {
            return std::get<std::reference_wrapper<ProgramOptions>>(it->second);
        } else {
            return *std::get<std::shared_ptr<ProgramOptions>>(it->second);
        }
    }
};

#endif