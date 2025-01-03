#ifndef __HYPERCUBE_OPTIONS_H__
#define __HYPERCUBE_OPTIONS_H__

#include <options/BasicOptions.h>
#include <stat_tests/hypercube/HypercubeTest.h>
#include <memory>
#include <cstdint>

class HypercubeOptions : public PartialOptions {
    public:
        HypercubeOptions() : PartialOptions("Hypercube test options") {
            namespace po = boost::program_options;
            addPartialVisible("dim,d", po::value<size_t>(&dim)->default_value(2), "hypercube dimension");
            addPartialVisible("nintervals,m", po::value<size_t>(&nIntervals)->default_value(100), "number of intervals per each dimension");
            addPartialVisible("nsamples,N", po::value<size_t>(&nSamples)->default_value(10000), "number of samples");
            addPartialVisible("stride,s", po::value<size_t>(&stride)->default_value(0), "stride, default is equal to dimension");
        }
        void update(const boost::program_options::variables_map& vm) override {
            if(stride == 0) {
                stride = dim;
            }
        }

        size_t dim;
        size_t nIntervals;
        size_t nSamples;
        size_t stride;
};

//todo: this class is redundant. The number of bits used to sample 1D index can be specified as an output of BitsRepack
/*class SamplingOptions : public PartialOptions {
    public:
        SamplingOptions() : PartialOptions("Sampling options") {
            namespace po = boost::program_options;
            addPartialVisible("samples_per_index", po::value<size_t>(&samples_per_index)->default_value(1), "how many samples after bitsrepack should be used to sample each 1D hyperindex");
        }
        size_t samples_per_index;
};*/

HypercubeProblem createHypercubeProblem(const HypercubeOptions hypercube_options) {
    return HypercubeProblem(hypercube_options.dim, hypercube_options.nIntervals, hypercube_options.stride, hypercube_options.nSamples);
}

std::shared_ptr<HypercubeSampler>  createHypercubeSampler(HypercubeProblem problem) {
    return std::make_shared<HypercubeSampler>(problem);
}



#endif
