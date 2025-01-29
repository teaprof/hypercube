#ifndef __HYPERCUBE_OPTIONS_H__
#define __HYPERCUBE_OPTIONS_H__

#include <options/BasicOptions.h>
#include <stat_tests/hypercube/HypercubeTest.h>
#include <memory>
#include <cstdint>
#include <options/cartesian_product.h>

class HypercubeOptions : public OptionsGroupStorage {
        std::vector<size_t> dim{1};
        std::vector<size_t> nIntervals{100};
        std::vector<size_t> nSamples{100000};
        std::vector<size_t> nSamplesPerCell{10};
        std::vector<size_t> stride{1};
    public:
        using cartesian_product_t = CartesianProduct2<std::vector<size_t>, std::vector<size_t>, std::vector<size_t>, std::vector<size_t>, std::vector<size_t>>;
        HypercubeOptions() : OptionsGroupStorage("Hypercube test options") {
            namespace po = boost::program_options;
            addPartialVisible("dim,d", po::value(&dim), "hypercube dimension");
            addPartialVisible("nintervals,m", po::value(&nIntervals), "number of intervals per each dimension");
            addPartialVisible("nsamples,N", po::value(&nSamples), "number of samples (should not be used with nSamplesPerCell)");
            addPartialVisible("nsamplespercell,M", po::value(&nSamplesPerCell), "number of samples per cell (should not be used with nSamples)");
            addPartialVisible("stride,s", po::value(&stride), "stride, default is equal to dimension");
        }
        void validate() override {
            if(!nSamplesPerCell.empty() && !nSamples.empty()) {
                throw std::runtime_error("--nSamples and --nSamplesPerCell should not be used simultaneously");
            }
        }
        void update(const boost::program_options::variables_map& vm) override {
            OptionsGroupStorage::update(vm);
            /*if(dim.empty()) {
                dim.push_back(2);
            }
            if(nIntervals.empty()) {
                nIntervals.push_back(100);
            }*/
            if(nSamples.empty() && nSamplesPerCell.empty()) {
                nSamplesPerCell.push_back(100);
            }
            /*if(stride.empty()) {
                stride.push_back(0);
            }*/
            this->options_combinations_.setSubspace<0>(dim);
            options_combinations_.setSubspace<1>(nIntervals);
            options_combinations_.setSubspace<2>(nSamples);
            options_combinations_.setSubspace<3>(nSamplesPerCell);
            options_combinations_.setSubspace<4>(stride);
        }

        HypercubeProblem description(const cartesian_product_t::index_t multi_index) const {            
            size_t dim = options_combinations_.get<0>(multi_index, 2);
            size_t nIntervals = options_combinations_.get<1>(multi_index, 100);
            std::optional<size_t> nSamples_opt = options_combinations_.get<3>(multi_index);
            std::optional<size_t> nSamplesPerCell_opt = options_combinations_.get<4>(multi_index);
            size_t stride = options_combinations_.get<4>(multi_index, dim);
            size_t nSamples =0;
            if(nSamples_opt.has_value()) {
                nSamples = nSamples_opt.value();
            } else {
                assert(nSamplesPerCell_opt.has_value());
                HypercubeProblem temp(dim, nIntervals, 0, 0);
                nSamples = nSamplesPerCell_opt.value()*temp.n_cells_total;
            }
            if(stride == 0) {
                stride = dim;
            }
            return HypercubeProblem(dim, nIntervals, stride, nSamples);
        }

        decltype(auto) begin() const {
            auto f = std::bind(&HypercubeOptions::description, this, std::placeholders::_1);
            return IteratorWithMapping(options_combinations_.begin(), f);
        }
        decltype(auto) end() const {
            auto f = std::bind(&HypercubeOptions::description, this, std::placeholders::_1);
            return IteratorWithMapping(options_combinations_.end(), f);
        }
    private:
        cartesian_product_t options_combinations_;
};


#endif
