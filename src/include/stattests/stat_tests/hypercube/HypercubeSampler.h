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
    }
    HypercubeSampler(const HypercubeSampler &other) : problem_(other.problem_), int_distribution_(other.int_distribution_), multi_index_{other.multi_index_} {}

    std::shared_ptr<DistributionSampler> copy() override {
        return std::make_shared<HypercubeSampler>(*this);
    }

    uint64_t operator()(RandomBitGenerator &rng) override {
        if (multi_index_.empty()) {
            //initial fill of the multi_index
            for (size_t n = 0; n < problem_.dim; n++) {
                auto v = int_distribution_(rng);
                multi_index_.push_back(v);
            }
        } else {
            assert(problem_.stride != 0);
            //update multi_index: add to the end new values and shift
            for (size_t n = 0; n < problem_.stride; n++) {
                multi_index_.pop_front(); /// \todo: use circular buffer
                auto v = int_distribution_(rng);
                multi_index_.push_back(v);
            }
        }
        return sub2ind(multi_index_);
    }
    double getProbability(size_t value, const RandomBitGenerator& rng) const override {
        double res = 1;
        const auto& multiindex = ind2sub(value);
        for(const auto& v : multiindex) 
            res *= int_distribution_.getProbability(v, rng);
        return res;
    }

    void discardN(uint64_t N, RandomBitGenerator &rng) override {
        // make the same job as the following loop:
        //      for(uint64_t n = 0; n < N; n++)
        //          this->operator()(rng);
        // but in more smart manner using `int_distribution_.discardN` function
        size_t steps_for_refill = intceil(problem_.dim, problem_.stride);
        if(N < steps_for_refill) {
            for(uint64_t n = 0; n < N; n++)
                this->operator()(rng);
            return;
        };
        int_distribution_.discard(N - steps_for_refill, rng);
        for(size_t n = 0; n < steps_for_refill; n++)
            this->operator()(rng);
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
    std::deque<uint64_t> multi_index_;
    
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
    size_t intceil(size_t divident, size_t divisor) {
        // equals to static_cast<size_t>(ceil(divident/divisor))
        return (divident + divisor - 1)/divisor;
    }
};



#endif