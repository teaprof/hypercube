#ifndef __HYPERCUBE_OPTIONS_H__
#define __HYPERCUBE_OPTIONS_H__

#include <options/BasicOptions.h>
#include <stat_tests/hypercube/HypercubeTest.h>
#include <memory>
#include <cstdint>

class HypercubeOptions : public OptionsGroupStorage {
    public:
        HypercubeOptions() : OptionsGroupStorage("Hypercube test options") {
            namespace po = boost::program_options;
            addPartialVisible("dim,d", po::value<size_t>(&dim)->default_value(2), "hypercube dimension");
            addPartialVisible("nintervals,m", po::value<size_t>(&nIntervals)->default_value(100), "number of intervals per each dimension");
            addPartialVisible("nsamples,N", po::value<size_t>(&nSamples)->default_value(10000), "number of samples");
            addPartialVisible("stride,s", po::value<size_t>(&stride)->default_value(0), "stride, default is equal to dimension");
            addPartialVisible("t", po::value<size_t>(&stride)->default_value(0), "test option");
        }
        void update(const boost::program_options::variables_map& vm) override {
            OptionsGroupStorage::update(vm);
            if(stride == 0) {
                stride = dim;
            }            
        }

        HypercubeProblem problem() {
            HypercubeProblem problem(dim, nIntervals, stride, nSamples);
            return problem;
        }
        size_t dim{1};
        size_t nIntervals{100};
        size_t nSamples{100000};
        size_t stride{1};
};


#endif
