#ifndef __HYPERCUBE_OPTIONS_H__
#define __HYPERCUBE_OPTIONS_H__

#include <ProgramOptionsHeavy.h>
//#include <stat_tests/hypercube/HypercubeSampler.h>
#include <app/options/cartesian_product.h>

class HypercubeOptions : public program_options_heavy::HeavyOptionsGroup {
    public:
        using cartesian_product_t = CartesianProduct2<std::vector<size_t>, std::vector<size_t>, std::vector<size_t>, std::vector<size_t>, std::vector<size_t>>;
        HypercubeOptions() : HeavyOptionsGroup("Hypercube test options") {            
            // TODO: uncomment
            //OptionsGroup::description<<"Most of this options can be used more than once. In such case all combinations will take play. ";
            //OptionsGroup::description<<"For example, \"-d2 -d3 -M10 -M500\" leads to four runs with parameters: -d2 -M10, -d2 -M500, -d3 -M10, -d3 -M500\n";
            namespace po = boost::program_options;
            std::stringstream default_str, nIntervals_str, nSamplesPerCell_str;
            default_str<<"hypercube dimension (default is "<<dim_default<<")";
            nIntervals_str<<"number of intervals per each dimension (default is "<<nIntervals_default<<")";
            nSamplesPerCell_str<<"number of samples per cell (should not be used with nSamples)(default is "<<nsamples_per_cell_default<<")";
            addPartial("dim,d", std::ref(dim), default_str.str().c_str());
            addPartial("nintervals,m", std::ref(nIntervals), nIntervals_str.str().c_str());
            addPartial("nsamples,N", std::ref(nSamples), "number of samples (should not be used with nSamplesPerCell), not used by default");
            addPartial("nsamplespercell,M", std::ref(nSamplesPerCell), nSamplesPerCell_str.str().c_str());
            addPartial("stride,s", std::ref(stride), "stride, default is equal to dimension");
        }
        void validate() override {
            // TODO: initialize nSamplesPerCell_v_ and  nSamples_v_
            if(!nSamplesPerCell_v_.empty() && !nSamples_v_.empty()) {
                throw std::runtime_error("--nSamples and --nSamplesPerCell should not be used simultaneously");
            }
        }
        void update(const boost::program_options::variables_map& vm) override {
            HeavyOptionsGroup::update(vm);
            // the default values for all parameters are set in `description` fcn            
            // TODO: initialize *_v_ variables
            options_combinations_.setSubspace<0>(dim_v_);
            options_combinations_.setSubspace<1>(nIntervals_v_);
            options_combinations_.setSubspace<2>(nSamples_v_);
            options_combinations_.setSubspace<3>(nSamplesPerCell_v_);
            options_combinations_.setSubspace<4>(stride_v_);
        }

        HypercubeProblem description(const cartesian_product_t::index_t multi_index) const {            
            size_t dim = options_combinations_.get<0>(multi_index, dim_default);
            size_t nIntervals = options_combinations_.get<1>(multi_index, nIntervals_default);
            std::optional<size_t> nSamples_opt = options_combinations_.get<2>(multi_index);
            std::optional<size_t> nSamplesPerCell_opt = options_combinations_.get<3>(multi_index, nsamples_per_cell_default);
            size_t stride = options_combinations_.get<4>(multi_index, dim);

            size_t nSamples = 0;
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
        std::vector<size_t> dim_v_;
        std::vector<size_t> nIntervals_v_;
        std::vector<size_t> nSamples_v_;
        std::vector<size_t> nSamplesPerCell_v_;
        std::vector<size_t> stride_v_;


        size_t dim{1};
        size_t nIntervals{100};
        size_t nSamples{100000};
        size_t nSamplesPerCell{10};
        size_t stride{1};
        static constexpr size_t dim_default = 2; // TODO: where this is used
        static constexpr size_t nIntervals_default = 100; // TODO: where this is used
        static constexpr size_t nsamples_per_cell_default = 100; // TODO: where this is used
};


#endif
