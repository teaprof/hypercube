#ifndef __HYPERCUBE_TEST_H__
#define __HYPERCUBE_TEST_H__

#include <stattests/stat_tests/hypercube/HypercubeProblem.h>
#include <librandom/distributions/Distribution.h>
#include <librandom/rng/dynamic/generators/RandomNumberWrapper.h>
#include <deque>


class HypercubeSampler : public DistributionSampler {
public:
    HypercubeSampler(const HypercubeProblem &problem) : problem_(problem), int_distribution_(problem.m_intervals_per_dim - 1) 
    {
        if(problem.stride == 0) {
            throw std::runtime_error("stride should be positive non-zero value");
        }
        n_cells_total_ = 1;
        for(size_t n = 0; n < problem_.dim; n++)
            n_cells_total_ *= problem_.m_intervals_per_dim;
    }
    HypercubeSampler(const HypercubeSampler &other) = default;

    std::shared_ptr<DistributionSampler> copy() override {
        return std::make_shared<HypercubeSampler>(*this);
    }

    uint64_t operator()(RandomBitGenerator &rng) override {
        if (multi_index_ == EMPTY_VALUE) {
            //initial fill of the multi_index
            multi_index_ = 0;
            for (size_t n = 0; n < problem_.dim; n++) {
                auto v = int_distribution_(rng);
                multi_index_ = v + multi_index_*problem_.m_intervals_per_dim;
            }
        } else {
            assert(problem_.stride != 0);
            size_t skip = 0;
            if(problem_.stride > problem_.dim) {
                skip = problem_.stride - problem_.dim;
                for(size_t n = 0; n < skip; n++)
                    int_distribution_.discard(rng);
            }
            //update multi_index: add to the end new values and shift
            for (size_t n = skip; n < problem_.stride; n++) {
                auto v = int_distribution_(rng);
                multi_index_ = v + multi_index_*problem_.m_intervals_per_dim;
                multi_index_ %= n_cells_total_;
            }
        }
        //return sub2ind(multi_index_);
        return multi_index_;
    }
    void initializeProbabilities(const RandomBitGenerator& rng) override {
        probabilities_.clear();
        if(problem_.dim > 1) {
            probabilities_.reserve(problem_.m_intervals_per_dim);
            for(size_t n = 0; n < problem_.m_intervals_per_dim; n++)
                probabilities_.push_back(int_distribution_.getProbability(n, rng));
        }
    }
    double getProbability(const RandomBitGenerator& rng, size_t linear_index) const override { 
        const auto& multiindex = ind2sub(linear_index);
        double res = 1;
        if(probabilities_.size() == 0) {
            for(const auto& v : multiindex) 
                res *= int_distribution_.getProbability(v, rng);
            return res;
        }
        assert(probabilities_.size() == problem_.m_intervals_per_dim);
        for(const auto& v : multiindex) 
            res *= probabilities_[v];
        return res;
    }

    void discardN(uint64_t N, RandomBitGenerator &rng) override {
        if(N == 0)
            return;
        if (multi_index_ == EMPTY_VALUE) {
            uint64_t skip = problem_.stride * N;
            int_distribution_.discardN(skip, rng);
            return;
        };
        // new rng offset relative to the position from which current multi_index was generated
        uint64_t new_rng_offset = problem_.stride * (N + 1); 
        // current rng relative offset
        uint64_t cur_rng_offset = problem_.dim;
        if(new_rng_offset > problem_.dim) {
            int_distribution_.discardN(new_rng_offset - cur_rng_offset, rng);
            multi_index_ = EMPTY_VALUE;
        } else {
            for(uint64_t n = 0; n < N; n++)
                operator()(rng);
        }
    }
    uint64_t max() const override {
        return problem_.n_cells_total - 1;
    }

    uint64_t getNumberOfRngCalls(uint64_t numberOfSampleCalls) const override {
        assert(numberOfSampleCalls > 0);
        return problem_.dim + (numberOfSampleCalls-1)*problem_.stride;
    }

protected:
    const HypercubeProblem problem_;
    UniformIntDistributionRough int_distribution_;
    static constexpr uint64_t EMPTY_VALUE = std::numeric_limits<uint64_t>::max();
    uint64_t multi_index_{EMPTY_VALUE};
    uint64_t n_cells_total_;

    std::vector<double> probabilities_;
    //probabilities[i] is the probability that single index falls into i-th interval
    
    template<class T>
    size_t sub2ind(const T &multiindex) const {
        size_t idx = 0;
        assert(multiindex.size() == problem_.dim);
        for (auto& it : multiindex) {
            idx = idx*problem_.m_intervals_per_dim + it;
        }
        return idx;
    }

    std::vector<size_t> ind2sub(uint64_t idx) const {
        std::vector<size_t> res;
        res.assign(problem_.dim, 0);
        for(size_t n = 0; n < problem_.dim; n++) {
            size_t i = idx % problem_.m_intervals_per_dim;
            res[res.size() - n - 1] = i;
            idx /= problem_.m_intervals_per_dim; 
        }
        return res;
    }
    size_t divintceil(size_t divident, size_t divisor) {
        // equals to static_cast<size_t>(ceil(divident/divisor))
        return (divident + divisor - 1)/divisor;
    }
};



#endif