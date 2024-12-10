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
            visible_.add_options()(name.c_str(), s, description.c_str());
        }
        virtual void validate() {
            //nothing to do
        }        
    private:
        //These vars are used only for parsing arguments
        boost::program_options::options_description partial_;
        boost::program_options::positional_options_description  positional_;        

        //These vars are used only for printing help message
        boost::program_options::options_description visible_;
        boost::program_options::options_description hidden_;
};

class BasicOptions : public PartialOptions {
    public:
        BasicOptions() : PartialOptions("Basic options") {
            addPartialVisible("help", "produce help");
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

class Options {
    public:
        void parse(int argc, char* argv[]) {

        }
        void validate() {
            for(auto it : options) 
                it.get().validate();
        }
        void help() {
            auto visible_options = visible();
            std::cout<<visible_options<<std::endl;
        }
        void add(PartialOptions& partial_options) {
            options.push_back(partial_options);
        }
        boost::program_options::options_description partial() {
            boost::program_options::options_description res;        
            for(auto it : options)
                res.add(it.get().partial());
            return res;
        }
        boost::program_options::options_description visible() {
            boost::program_options::options_description res;        
            for(auto it : options)
                res.add(it.get().visible());
            return res;

        }
        boost::program_options::options_description hidden() {
            return {};
        }
        boost::program_options::positional_options_description positional() {
            return {};
        }
        boost::program_options::options_description all() {
            boost::program_options::options_description all;
            for(auto it : options) {
                all.add(it.get().partial());
            }
            return all;
        }

    private:
        std::vector<std::reference_wrapper<PartialOptions>> options;
};

class HypercubeTestOptions {

};


#endif