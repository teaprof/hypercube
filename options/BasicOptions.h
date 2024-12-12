#ifndef BASIC_OPTIONS_H
#define BASIC_OPTIONS_H

#include <boost/program_options.hpp>
#include <thread>
#include <vector>
#include <iostream>

class PartialOptions {
    public:
        PartialOptions(std::string caption) : visible_(caption) {}
        boost::program_options::options_description partial() {
            return partial_;
        }
        boost::program_options::options_description visible() {
            return visible_;
        }
        boost::program_options::options_description hidden() {
            return hidden_;
        }
        boost::program_options::positional_options_description positional() {
            return positional_;
        }
        template<class...Args>
        void addPartialVisible(Args...args) {
            partial_.add_options()(args...);
            visible_.add_options()(args...);
        }
        void addPositionalHidden(std::string name, int count, const boost::program_options::value_semantic* s) {
            positional_.add(name.c_str(), count);
            partial_.add_options()(name.c_str(), s);
        }
        void addPositionalVisible(std::string name, int count, const boost::program_options::value_semantic* s, std::string description) {
            addPositionalHidden(name, count, s);
            partial_.add_options()(name.c_str(), s);
            visible_.add_options()(name.c_str(), s, description.c_str());
        }
        virtual void validate() {
            //nothing to do
        }        

        //These vars are used only for parsing arguments
        boost::program_options::options_description partial_;
        boost::program_options::positional_options_description  positional_;        

        //These vars are used only for printing help message
        boost::program_options::options_description visible_;
        boost::program_options::options_description hidden_;

        boost::program_options::variables_map vm;
};

class BasicOptions : public PartialOptions {
    public:
        BasicOptions() : PartialOptions("Basic options") {
            namespace po = boost::program_options;
            addPartialVisible("help", po::value<bool>(&need_help)->default_value(false), "produce help");
        }
        bool need_help;
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
        size_t defaultThreads_, nthreads_;
};


class HypercubeOptions : public PartialOptions {
    public:
        HypercubeOptions() : PartialOptions("Hypercube test options") {
            namespace po = boost::program_options;
            addPartialVisible("dim,d", po::value<size_t>(&dim), "hypercube dimension");
            addPartialVisible("nintervals,m", po::value<size_t>(&nIntervals)->default_value(100), "hypercube dimension");
            addPartialVisible("nsamples,N", po::value<size_t>(&nIntervals)->default_value(100), "hypercube dimension");
        }
        size_t dim;
        size_t nIntervals;
        size_t nSamples;
};

class Options : protected PartialOptions {
    public:
        Options() : PartialOptions("") {}
        virtual void addGroup(PartialOptions& options) {
            partial_.add(options.partial_);
            visible_.add(options.visible_);
            hidden_.add(options.hidden_);
            options_.push_back(std::ref(options));
            if(options.positional_.max_total_count() != 0) {
                //positional arguments should be added only once
                assert(positional_.max_total_count() == 0);
                positional_ = options.positional_;
            }
        }
        virtual void parse(int argc, char* argv[]) {
            namespace po = boost::program_options;
            po::store(po::command_line_parser(argc, argv).options(partial_).positional(positional_).run(), vm);
            boost::program_options::notify(vm);
        }
        void validate() {
            for(auto it : options_) 
                it.get().validate();
        }
        void help() {
            std::cout<<visible_<<std::endl;
        }
    private:
        std::vector<std::reference_wrapper<PartialOptions>> options_;
};

class HypercubeTestOptions {

};


#endif