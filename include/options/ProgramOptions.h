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

class DocumentedOptionsGroup : public TextSectionAutoBody {
    public:    
        std::stringstream body() const override {
            return detailedList();
        }

        void setGroupName(std::string str) {
            group_name_ = std::move(str);
        }
        const std::string& groupName() const {
            return group_name_;
        }
        virtual std::stringstream detailedList() const = 0;
    protected:
        std::string group_name_;
        //These values are used to generate documentation
        std::string title_;
        std::string header_;
        std::string footer_;
};

class OptionsGroupStorage : public DocumentedOptionsGroup {
    /// \todo: rename this class
    public:
        static constexpr uint16_t text_width = 140;
        
        OptionsGroupStorage(std::string group_name) : visible(group_name, text_width) 
        {
            setGroupName(group_name);
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

        std::stringstream detailedList() const override {
            std::stringstream res;
            res << visible;
            return res;
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
    private:
        //bool not_specified_{true}; // true if none of the positional or partial options are specified
};

class OptionsParser  {
public:
    virtual bool parse(int argc, const char* argv[]) = 0;
    virtual void validate() = 0;
    virtual void update(const boost::program_options::variables_map& vm) = 0;
};

class ProgramOptions : public OptionsParser, public TextSection {
    // This class can parse the list of options and print the help message
    // Use this class for simple set of command line options like: 
    // programname --arg1 --arg2 10 -zxc -v 20 input.txt output.txt
    public:
        ProgramOptions() = default;
        virtual void addGroup(std::shared_ptr<OptionsGroupStorage> options) {
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
            return true;
        }
        void validate() override {
            for(auto it : options_) 
                it->validate();
        }
        void  update(const boost::program_options::variables_map& vm) override {

        }
        std::vector<std::shared_ptr<TextSectionBase>> subsections() const override {
            std::vector<std::shared_ptr<TextSectionBase>>  res;
            for(auto it : options_) {
                res.push_back(it);
            }
            return res;
        }
        /*std::string shortList() const  {
            std::stringstream str;
            for(auto it : options_) {
                str<<it.get().groupName()<<" ";
            }
            return str.str();
        }*/
        const std::vector<std::shared_ptr<OptionsGroupStorage>> options() const {
            return options_;
        }
    private:
        std::vector<std::shared_ptr<OptionsGroupStorage>> options_;
    };


class ProgramModesOptions : public OptionsParser, public TextSection {
    /// \todo: don't inherit OptionsGroupStorage here
    using value_t = std::variant<std::reference_wrapper<ProgramOptions>, std::shared_ptr<ProgramOptions>>;
    using modes_t = std::map<std::string, value_t>;
    public:
    ProgramModesOptions() : OptionsParser() {}    
    ProgramOptions& push_back(const std::string& mode_name, ProgramOptions& val) {
        auto res = modes_.emplace(mode_name, std::ref(val));
        if(!res.second) {
            throw std::runtime_error("The specified mode_name is already present in ProgramModesOptions");
        }
        modes_order_.push_back(modes_.find(mode_name));
        return getref(res.first);
    }
    ProgramOptions& push_back(const std::string& mode_name, std::shared_ptr<ProgramOptions> val) {
        auto res = modes_.emplace(mode_name, val);
        if(!res.second) {
            throw std::runtime_error("The specified mode_name is already present in ProgramModesOptions");
        }
        modes_order_.push_back(modes_.find(mode_name));
        return getref(res.first);
    }
    ProgramOptions& at(const std::string& mode_name) {
        auto pos = modes_.find(mode_name);
        if(pos != modes_.end()) {
            throw std::out_of_range("the requested mode_name is not contained in the mode list");
        }
        return getref(pos);
    }    
    ProgramOptions& operator[](const std::string& mode_name) {
        auto pos = modes_.find(mode_name);
        if(pos == modes_.end()) {
            pos = modes_.emplace(mode_name, std::make_shared<ProgramOptions>()).first;
        }
        return getref(pos);
    }    
    ProgramOptions& defaultMode() {
        return (*this)[default_mode_name_];
    }
    void setDefaultModeName(const std::string& mode_name) {
        default_mode_name_ = mode_name;
    }
    ProgramOptions& selectedMode() {
        return getref(selected_mode_);
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
        getref(selected_mode_).parse(argc, argv);
        return true;
    }
    void validate() override {
    }
    void  update(const boost::program_options::variables_map& vm) override {
    }
    void showDefaultModeName(bool flag) {
        show_default_mode_name_ = flag;
    }

    class TextSectionWithDetails : public TextSection {
    public:
        TextSectionWithDetails(const ProgramModesOptions* parent) : parent_(parent) {}
        std::vector<std::shared_ptr<TextSectionBase>>  subsections() const override {
            std::vector<std::shared_ptr<TextSectionBase>> res;
            for(auto& mode : parent_->modes_order_) {
                auto ptr = parent_->getptr(mode);
                if(ptr) {
                    res.push_back(ptr);
                }
            }
            return res;
        }
    private:
        const ProgramModesOptions* parent_;
    };
    std::vector<std::shared_ptr<TextSectionBase>>  subsections() const override {
        auto usage = std::make_shared<TextSection>();
        usage->title_<<"Usage:\n";
        for(auto& mode : modes_order_) {            
            usage->header_<<"\t"<<shortHelp(mode)<<"\n";
        }

        auto description = std::make_shared<TextSection>();
        description->title_<<"Detailed description:";
        description->header_<<program_description;

        auto options = std::make_shared<TextSectionWithDetails>(this);
        options->title_<<"Details:";

        std::vector<std::shared_ptr<TextSectionBase>> res;
        res.push_back(std::move(usage));
        res.push_back(std::move(description));
        res.push_back(std::move(options));
        return res;
    }
    std::string program_description;
    std::string exename{"hypercube"};

    std::string shortHelp(modes_t::iterator it) const {
        std::stringstream str;
        str<<exename<<" ";
        if(it->first == default_mode_name_) {
            if(show_default_mode_name_) {
                str<<"["<<it->first<<"] ";
            }
        } else {
            str<<it->first<<" ";
        }
        const ProgramOptions& opts = getref(it);
        for(auto group : opts.options()) {
            str<<"["<<group->groupName()<<"] ";
        }        
        return str.str();
    }

private:
    modes_t modes_;    
    std::vector<modes_t::iterator> modes_order_; //order of modes_ for printing purpose
    modes_t::iterator selected_mode_;
    std::string default_mode_name_{"default"};
    bool show_default_mode_name_{true};
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
    const std::shared_ptr<ProgramOptions> getptr(modes_t::const_iterator it) const {
        if(std::holds_alternative<std::reference_wrapper<ProgramOptions>>(it->second)) {
            return nullptr;
        } else {
            return std::get<std::shared_ptr<ProgramOptions>>(it->second);
        }
    }
};
#endif