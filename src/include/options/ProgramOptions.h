#ifndef __MODE_OPTIONS_H__
#define __MODE_OPTIONS_H__

#include <options/prettyprinter.h>

#include <map>
#include <string> 
#include <sstream>
#include <iostream>
#include <variant>
#include <locale>
#include <set>

#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>

class OptionsGroup {
    public:
        static constexpr uint16_t text_width = 140;
        
        OptionsGroup(std::string group_name) : visible(text_width) 
        {
            std::string lower;
            for(auto ch : group_name) {
                lower += std::tolower(ch);
            }
            setGroupName(lower);
        }
        
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

        virtual void validate() {
            //nothing to do
            //redefine this function in the derived class
        }
        virtual void update(const boost::program_options::variables_map& vm) {
            //nothing to do
            //redefine this function in the derived class
        }
    
        //These vars are used only for parsing arguments
        boost::program_options::options_description partial;
        boost::program_options::positional_options_description positional;        

        //These vars are used only for printing help message
        boost::program_options::options_description visible;
        boost::program_options::options_description hidden;         

        std::stringstream detailedList() const {
            std::stringstream res;
            res << visible;
            return res;
        }

        void setGroupName(std::string str) {
            group_name_ = std::move(str);
        }
        const std::string& groupName() const {
            return group_name_;
        }
        std::stringstream description;
    private:
        std::string group_name_;
        //bool not_specified_{true}; // true if none of the positional or partial options are specified
};

class OptionsParser  {
public:
    virtual bool parse(int argc, const char* argv[]) = 0;
    virtual void validate() = 0;
    virtual void update(const boost::program_options::variables_map& vm) = 0;
};

class ProgramOptions : public OptionsParser {
    // This class can parse the list of options and print the help message
    // Use this class for simple set of command line options like: 
    // programname --arg1 --arg2 10 -zxc -v 20 input.txt output.txt
    public:
        ProgramOptions() = default;
        virtual void addGroup(std::shared_ptr<OptionsGroup> options) {
            if(options->positional.max_total_count() != 0) {
                for(auto it : options_) {
                    //only one group of options is allowed to have positional arguments
                    assert(it->positional.max_total_count() == 0);
                }
            }
            options_.push_back(options);
        }
        bool parse(int argc, const char* argv[]) override {
            namespace po = boost::program_options;
            boost::program_options::options_description partial;
            boost::program_options::positional_options_description positional;        
            for(auto it : options_) {
                partial.add(it->partial);
                if(it->positional.max_total_count() != 0) {
                    //only one group of options is allowed to have positional arguments
                    assert(positional.max_total_count() == 0);
                    positional = it->positional;
                }
            }
            boost::program_options::variables_map vm;
            po::store(po::command_line_parser(argc, argv).options(partial).positional(positional).run(), vm);
            boost::program_options::notify(vm);
            for(auto it : options_) {
                it->update(vm);
            }
            activated = true;
            return true;
        }
        void validate() override {
            for(auto it : options_) 
                it->validate();
        }
        void  update(const boost::program_options::variables_map& vm) override {

        }
        const std::vector<std::shared_ptr<OptionsGroup>> options() const {
            return options_;
        }

        std::string title;
        std::string description;
        bool activated{false}; //becomes true when parse function succeeded
    private:
        std::vector<std::shared_ptr<OptionsGroup>> options_;
    };


class ProgramModesOptions : public OptionsParser {
    using value_t = std::shared_ptr<ProgramOptions>;
    using modes_t = std::map<std::string, value_t>;
    public:
    ProgramModesOptions() : OptionsParser() {}    
    std::shared_ptr<ProgramOptions> push_back(const std::string& mode_name, std::shared_ptr<ProgramOptions> val) {
        auto res = modes_.emplace(mode_name, val);
        if(!res.second) {
            throw std::runtime_error("The specified mode_name is already present in ProgramModesOptions");
        }
        modes_order_.push_back(modes_.find(mode_name));
        return res.first->second;
    }
    std::shared_ptr<ProgramOptions> operator[](const std::string& mode_name) {
        auto pos = modes_.find(mode_name);
        if(pos == modes_.end()) {
            pos = modes_.emplace(mode_name, std::make_shared<ProgramOptions>()).first;
        }
        return pos->second;
    }    
    std::shared_ptr<ProgramOptions> defaultMode() {
        return (*this)[default_mode_name_];
    }
    void setDefaultModeName(const std::string& mode_name) {
        default_mode_name_ = mode_name;
    }
    std::shared_ptr<ProgramOptions> selectedMode() {
        return selected_mode_->second;
    }
    const std::string& selectedModeName() {
        return selected_mode_->first;
    }
    bool parse(int argc, const char* argv[]) override {
        assert(!modes_.empty());
        if(argc >= 2) {
            const char* first_arg = argv[1];
            selected_mode_ = modes_.find(first_arg);
            if(selected_mode_ != modes_.end()) {
                argc--;
                argv++;
            } else {
                // Case: mode is unknown, fallback to default mode
                selected_mode_ = modes_.find(default_mode_name_);
                assert(selected_mode_ != modes_.end());
            }
        } else {
            // Case: no options specified, fallback to default mode
            selected_mode_ = modes_.find(default_mode_name_);
            assert(selected_mode_ != modes_.end());
        };
        selected_mode_->second->parse(argc, argv);
        activated = true;
        return true;
    }
    void validate() override {
    }
    void  update(const boost::program_options::variables_map& vm) override {
    }
    void showDefaultModeName(bool flag) {
        show_default_mode_name_ = flag;
    }

    std::string program_description;
    std::string exename{"hypercube"};
    bool activated{false}; //becomes true when parse function succeeded
private:
    modes_t modes_;    
    std::vector<modes_t::iterator> modes_order_; //order of modes_ for printing purpose
    modes_t::iterator selected_mode_;
    std::string default_mode_name_{"default"};
    bool show_default_mode_name_{true};
    friend class ProgramModesOptionsPrinter;
};

class ProgramModesOptionsPrinter {
    public:
    std::shared_ptr<TextSectionBase> print(ProgramModesOptions& pmo) {
        auto res = std::make_shared<TextSection>();
        auto usage = std::make_shared<TextSection>();
        usage->title_<<"Usage:\n";
        for(auto& mode : pmo.modes_order_) {
            usage->header_<<"\t"<<shortHelp(pmo, mode)<<"\n";
        }

        auto description = std::make_shared<TextSection>();
        description->title_<<"Detailed description:";
        description->header_<<pmo.program_description;

        auto details = std::make_shared<TextSection>();
        details->title_<<"Details:";
        for(auto& mode : pmo.modes_order_) {
            auto ptr = mode->second;
            for(auto &it : print(*ptr)) {
                details->subsections_.push_back(it);
            }
        }

        res->subsections_.push_back(usage);
        res->subsections_.push_back(description);
        res->subsections_.push_back(details);
        return res;
    }
    std::string shortHelp(ProgramModesOptions& pmo, ProgramModesOptions::modes_t::iterator it) const {
        std::stringstream str;
        str<<pmo.exename<<" ";
        if(it->first == pmo.default_mode_name_) {
            if(pmo.show_default_mode_name_) {
                str<<"["<<it->first<<"] ";
            }
        } else {
            str<<it->first<<" ";
        }
        const std::shared_ptr<ProgramOptions> opts = it->second;
        for(auto group : opts->options()) {
            str<<"["<<group->groupName()<<"] ";
        }        
        return str.str();
    }
    std::vector<std::shared_ptr<TextSectionBase>> print(ProgramOptions& pmo) {
        std::vector<std::shared_ptr<TextSectionBase>>  res;
        //res->setTitle(pmo.title);
        //res->setHeader(pmo.description);
        for(auto it : pmo.options()) {
            if(!options_groups_printed_already_.contains(it->groupName())) {
                res.push_back(print(*it));
                options_groups_printed_already_.insert(it->groupName());
            }
        }
        return res;
    }
    std::shared_ptr<TextSectionBase> print(OptionsGroup& grp) const {
        auto res = std::make_shared<TextSection>();
        res->setTitle(grp.groupName());
        res->setHeader(grp.description.view());
        res->body_<<grp.detailedList().view();
        return res;
    }    
    std::set<std::string> options_groups_printed_already_;
};

#endif