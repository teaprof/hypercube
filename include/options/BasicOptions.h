#ifndef BASIC_OPTIONS_H
#define BASIC_OPTIONS_H

#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <thread>
#include <vector>
#include <iostream>

class PartialOptions {
    public:
        static constexpr uint16_t text_width = 140;
        PartialOptions(std::string caption) : visible(caption, text_width) {}        
        template<class...Args>
        auto addPartialVisible(Args ... args) {
            auto option = boost::make_shared<boost::program_options::option_description>(args...);            
            partial.add(option);
            visible.add(option);
            return option;
        }
        void addPositionalHidden(std::string name, int count, const boost::program_options::value_semantic* s) {
            positional.add(name.c_str(), count);
            partial.add_options()(name.c_str(), s);
        }
        auto addPositionalVisible(std::string name, int count, const boost::program_options::value_semantic* s, std::string description) {
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
        //These vars are used only for parsing arguments
        boost::program_options::options_description partial;
        boost::program_options::positional_options_description  positional;        

        //These vars are used only for printing help message
        boost::program_options::options_description visible;
        boost::program_options::options_description hidden;        
};

class BasicOptions : public PartialOptions {
    public:
        BasicOptions() : PartialOptions("Basic options") {
            namespace po = boost::program_options;
            //addPartialVisible("help", new po::untyped_value(true), "produce help");
            //addPartialVisible("help", po::value(&need_help), "produce help");
            addPartialVisible("help", po::bool_switch(&need_help), "produce help");
        }
        void update(const boost::program_options::variables_map& vm) override {            
            //need_help = vm.count("help") > 0;
        }
        bool needHelp() const {
            return need_help;
        }
    private:
        bool need_help;
};

class MultithreadOptions : public PartialOptions {
    public:
        MultithreadOptions() : PartialOptions("multithreading options") {
            namespace po = boost::program_options;
            concurency = std::thread::hardware_concurrency();
            std::stringstream str;
            str<<"number of threads, if not set then std::threads::hardware_concurrency will be used ["<<concurency<<" on this machine]";
            addPartialVisible("nthreads,t", po::value(&nthreads_), str.str().c_str());
        }
        size_t nThreads() {
            if(nthreads_) {
                return *nthreads_;
            }
            return concurency;
        }
    private:
        size_t concurency;
        boost::optional<size_t> nthreads_;
};

class ProgramOptions : protected PartialOptions {
    public:
        ProgramOptions() : PartialOptions("") {}
        virtual void addGroup(PartialOptions& options) {
            partial.add(options.partial);
            visible.add(options.visible);
            hidden.add(options.hidden);
            options_.push_back(std::ref(options));
            if(options.positional.max_total_count() != 0) {
                //positional arguments should be added only once
                assert(positional.max_total_count() == 0);
                positional = options.positional;
            }
        }
        virtual void parse(int argc, char* argv[]) {
            namespace po = boost::program_options;
            po::store(po::command_line_parser(argc, argv).options(partial).positional(positional).run(), vm);
            boost::program_options::notify(vm);
            for(auto it : options_) {
                it.get().update(vm);
                it.get().validate();
            }            
        }
        void validate() {
            for(auto it : options_) 
                it.get().validate();
        }
        void help() const {
            std::cout<<visible<<std::endl;
        }
    private:
        std::vector<std::reference_wrapper<PartialOptions>> options_;
        boost::program_options::variables_map vm;
};


#endif