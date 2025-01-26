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
            addPartialVisible("dim,d", po::value(&dim), "hypercube dimension");
            addPartialVisible("nintervals,m", po::value(&nIntervals), "number of intervals per each dimension");
            addPartialVisible("nsamples,N", po::value(&nSamples), "number of samples (should not be used with nSamplesPerCell)");
            addPartialVisible("nsamplespercell,M", po::value(&nSamplesPerCell), "number of samples per cell (should not be used with nSamples)");
            addPartialVisible("stride,s", po::value(&stride), "stride, default is equal to dimension");
        }
        void update(const boost::program_options::variables_map& vm) override {
            OptionsGroupStorage::update(vm);
            if(dim.empty()) {
                dim.push_back(2);
            }
            if(nIntervals.empty()) {
                nIntervals.push_back(100);
            }
            if(nSamples.empty() && nSamplesPerCell.empty()) {
                nSamplesPerCell.push_back(100);
            }
            if(stride.empty()) {
                stride.push_back(0);
            }
        }
        void validate() override {
            if(!nSamplesPerCell.empty() && !nSamples.empty()) {
                throw std::runtime_error("--nSamples and --nSamplesPerCell should not be used simultaneously");
            }
        }

        /*HypercubeProblem problem() {
            HypercubeProblem problem(dim, nIntervals, stride, nSamples);
            return problem;
        }*/
        struct HypercubeProblemIterator {
            const HypercubeOptions& opts;
            size_t dim_idx;
            size_t nIntervals_idx;
            size_t nSamples_idx;
            size_t nSamplesPerCell_idx;
            size_t stride_idx;
            bool end_reached;
            HypercubeProblem operator*() const {
                size_t dim = opts.dim[dim_idx];
                size_t nIntervals = opts.nIntervals[nIntervals_idx];
                size_t nSamples = 0;
                if(!opts.nSamples.empty()) {
                    nSamples = opts.nSamples[nSamples_idx];
                } else {
                    assert(!opts.nSamplesPerCell.empty());
                    HypercubeProblem temp(dim, nIntervals, 0, 0);
                    nSamples = opts.nSamplesPerCell[nSamplesPerCell_idx]*temp.n_cells_total;
                }
                size_t stride = opts.stride[stride_idx];
                if(stride == 0) {
                    stride = dim;
                }
                return HypercubeProblem(dim, nIntervals, stride, nSamples);
            }
            void operator++() {
                if(++dim_idx == opts.dim.size()) {
                    dim_idx = 0;
                    if(++nIntervals_idx == opts.nIntervals.size()) {
                        nIntervals_idx = 0;
                        if(++nSamples_idx >= opts.nSamples.size()) {
                            nSamples_idx = 0;
                            if(++nSamplesPerCell_idx >= opts.nSamplesPerCell.size()) {
                                nSamplesPerCell_idx = 0;
                                if(++stride_idx == opts.stride.size()) {
                                    stride_idx = 0;
                                    end_reached = true;
                                }
                            }
                        }
                    };
                }
            }
            bool operator==(const HypercubeProblemIterator& other) const {
                return dim_idx == other.dim_idx && nIntervals_idx == other.nIntervals_idx && nSamples_idx == other.nSamples_idx && stride_idx == other.stride_idx && end_reached == other.end_reached;
            }
            bool operator!=(const HypercubeProblemIterator& other) const {
                return !(*this == other);
            }
        };
        HypercubeProblemIterator begin() const {
            return HypercubeProblemIterator{*this, 0, 0, 0, 0, 0, false};
        }
        HypercubeProblemIterator end() const {
            return HypercubeProblemIterator{*this, 0, 0, 0, 0, 0, true};
        }

        std::vector<size_t> dim{1};
        std::vector<size_t> nIntervals{100};
        std::vector<size_t> nSamples{100000};
        std::vector<size_t> nSamplesPerCell{10};
        std::vector<size_t> stride{1};
};


#endif
