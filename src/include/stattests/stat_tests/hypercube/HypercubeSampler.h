#ifndef __HYPERCUBE_TEST_H__
#define __HYPERCUBE_TEST_H__

#include <stattests/stat_tests/hypercube/HypercubeProblem.h>
#include <rnglib/distributions/Distribution.h>
#include <rnglib/rng/dynamic/generators/RandomNumberWrapper.h>
#include <deque>


class HypercubeSampler : public DistributionSampler {
public:
    HypercubeSampler(const HypercubeProblem &problem) : problem_(problem), int_distribution_(problem.m_intervals_per_dim - 1) {}
    HypercubeSampler(const HypercubeSampler &other) : problem_(other.problem_), int_distribution_(other.int_distribution_), multi_index_{other.multi_index_} {}

    std::shared_ptr<DistributionSampler> copy() override {
        return std::make_shared<HypercubeSampler>(*this);
    }

    uint64_t operator()(RandomBitGenerator &rng) override {
        if (multi_index_.empty()) {
            for (size_t n = 0; n < problem_.dim; n++) {
                auto v = int_distribution_(rng);
                multi_index_.push_back(v);
            }
        } else {
            for (size_t n = 0; n < problem_.stride; n++) {
                multi_index_.pop_front();
                auto v = int_distribution_(rng);
                multi_index_.push_back(v);
            }
        }
        return sub2ind(multi_index_);
    }    

    void discardN(size_t N, RandomBitGenerator &rng) override {
        for (size_t n = 0; n < N; n++)
            (*this)(rng);
    }
    uint64_t max() override {
        return problem_.n_cells_total - 1;
    }

protected:
    const HypercubeProblem problem_;
    UniformIntDistribution int_distribution_;
    std::deque<uint64_t> multi_index_;
    template<class T>
    size_t sub2ind(const T &multiindex) {
        size_t idx = 0;
        assert(multiindex.size() == problem_.dim);
        for (auto& it : multiindex) {
            idx = idx*problem_.m_intervals_per_dim + it;
        }
        return idx;
    }
};



#endif