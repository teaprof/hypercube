#ifndef BASIC_OPTIONS_H
#define BASIC_OPTIONS_H

#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>
#include <thread>
#include <vector>
#include <iostream>

class PartialOptions {
    public:
        PartialOptions(std::string caption) : visible(caption) {}        
        template<class...Args>
        void addPartialVisible(Args ... args) {
            auto option = boost::make_shared<boost::program_options::option_description>(args...);
            partial.add(option);
            visible.add(option);
        }
        void addPositionalHidden(std::string name, int count, const boost::program_options::value_semantic* s) {
            positional.add(name.c_str(), count);
            partial.add_options()(name.c_str(), s);
        }
        void addPositionalVisible(std::string name, int count, const boost::program_options::value_semantic* s, std::string description) {
            addPositionalHidden(name, count, s);
            auto option = boost::make_shared<boost::program_options::option_description>(name.c_str(), s, description.c_str());
            partial.add(option);
            visible.add(option);            
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
            addPartialVisible("help", new po::untyped_value(true), "produce help");
        }
        virtual void update(const boost::program_options::variables_map& vm) {
            need_help = vm.count("help") > 0;
        }
        bool need_help{false};
        
};

class MultithreadOptions : public PartialOptions {
    public:
        MultithreadOptions() : PartialOptions("multithreading options") {
            namespace po = boost::program_options;
            defaultThreads_ = 0;
            std::stringstream str;
            auto concurency = std::thread::hardware_concurrency();
            str<<"number of threads, if 0 then std::threads::hardware_concurrency will be used ["<<concurency<<" on this machine]";
            addPartialVisible("nthreads,t", po::value<size_t>(&nthreads_)->default_value(defaultThreads_), str.str().c_str());
        }
        size_t nThreads() {
            if(nthreads_ == 0) {
                return std::thread::hardware_concurrency();
            }
            return nthreads_;
        }
    private:
        size_t defaultThreads_, nthreads_;
};


class HypercubeOptions : public PartialOptions {
    public:
        HypercubeOptions() : PartialOptions("Hypercube test options") {
            namespace po = boost::program_options;
            addPartialVisible("dim,d", po::value<size_t>(&dim)->default_value(2), "hypercube dimension");
            addPartialVisible("nintervals,m", po::value<size_t>(&nIntervals)->default_value(100), "number of intervals per each dimension");
            addPartialVisible("nsamples,N", po::value<size_t>(&nSamples)->default_value(10000), "number of samples");
        }
        size_t dim;
        size_t nIntervals;
        size_t nSamples;
};

class SubtaskOptions : public PartialOptions {
    public:
        SubtaskOptions() : PartialOptions("Subtast options") {
            namespace po = boost::program_options;
            addPartialVisible("nsubtasks", po::value<size_t>(&nSubtasks)->default_value(1), "total count of subtasks");
            addPartialVisible("subtask", po::value<size_t>(&curSubtask)->default_value(0), "number of the current subtask");
        }
        size_t nSubtasks;
        size_t curSubtask;
};

class Options : protected PartialOptions {
    public:
        Options() : PartialOptions("") {}
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
        void help() {
            std::cout<<visible<<std::endl;
        }
    private:
        std::vector<std::reference_wrapper<PartialOptions>> options_;
        boost::program_options::variables_map vm;
};

class HypercubeTestOptions {

};


#endif