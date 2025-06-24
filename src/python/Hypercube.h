#ifndef __PYTHON_HYPERCUBE_H__
#define __PYTHON_HYPERCUBE_H__

//#include <singlerun.h>
#include <options/RngOptions.h>
#include <options/HypercubeOptions.h>

namespace py {

class RNGOptions {
    public:
        void setSeed(size_t seed) {
            options_.seed = seed;
        }
        void setName(const std::string& name) {
            options_.name = name;
        }
        void setNumber(size_t number) {
            options_.number = number;
        }
        void setOffset(size_t offset) {
            options_.offset = offset;
        }
    private:
        ::RNGOptions options_;
};

class BitsRepackOptions {
    public:
    private:
    bool src_little_endian_{true};
    bool dst_little_endian_{true};
    size_t dst_bits_per_sample_{0}; // 0 means that the value is not set
};

class HypercubeOptions {
    public:
        void setDim(size_t dim) {
            options_.dim = dim;
        }
        void setNIntervals(size_t nIntervals) {
            options_.nIntervals = nIntervals;
        }
        void setNSamples(size_t nSamples) {
            options_.nSamples = nSamples;
        }
        void setNSamplesPerCell(size_t nSamplesPerCell) {
            options_.nSamplesPerCell = nSamplesPerCell;
        }
        void setStride(size_t stride) {
            options_.stride = stride;
        }
    private:
        ::HypercubeOptions options_;

};

class SubtaskOptions {
    public:
        void setNSubtasks(size_t nSubtasks) {
            options_.nSubtasks = nSubtasks;
        }
        void setCurSubtask(size_t curSubtask) {
            options_.curSubtask = curSubtask;
        }
    private:
        ::SubtaskOptions options_;
};


class Hypercube {
    public:
        Hypercube(RNGOptions& rngopts, HypercubeOptions& hopts, SubtaskOptions& subtaskopts) {

        }
    private:
        ::Hypercube hypercube_;

};

}


#endif // __PYTHON_ENCODER_H__