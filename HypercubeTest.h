#ifndef __HYPERCUBE_TEST_H__
#define __HYPERCUBE_TEST_H__
#include "IndexGenerator.h"
#include "StatisticalTestBase.h"

class HypercubeProblem : public Chi2BasedProblem {
public:
    HypercubeProblem(size_t dim, size_t m_intervals_per_dim, size_t N) {
        this->dim = dim;
        this->m_intervals_per_dim = m_intervals_per_dim;
        this->N = N;
        m_intervals_total = 1;
        for (size_t k = 0; k < dim; k++)
            m_intervals_total *= m_intervals_per_dim;
    }
    size_t dim;
    size_t m_intervals_per_dim;
};

class HypercubeSampler : public DistributionSampler {
public:
    HypercubeSampler(const HypercubeProblem &hypercube_problem) : problem_(hypercube_problem) {}

    virtual size_t operator()(RandomBitGenerator &rng) = 0;

    virtual size_t max() { return problem_.m_intervals_total - 1; }
    virtual void discardN(size_t N, RandomBitGenerator &rng) {
        for (size_t n = 0; n < N; n++)
            (*this)(rng);
    }

protected:
    const HypercubeProblem problem_;
    size_t sub2ind(const std::vector<size_t> &multiindex) {
        size_t idx = 0;
        assert(multiindex.size() == problem_.dim);
        for (size_t k = 0; k < problem_.dim; k++) {
            idx = problem_.m_intervals_per_dim + multiindex[k];
        }
        return idx;
    }
};

class HypercubeIntSampler : public HypercubeSampler {
public:
    HypercubeIntSampler(HypercubeProblem problem, size_t stride) : HypercubeSampler(problem), stride_(stride){};
    virtual size_t operator()(RandomBitGenerator &rng) {
        if (multiindex.empty()) {
            for (size_t n = 0; n < problem_.dim; n++)
                multiindex.push_back(distr(problem_.m_intervals_per_dim, rng));
        } else {
            for (size_t n = 0; n < stride_; n++) {
                multiindex.push_back(distr(problem_.m_intervals_per_dim, rng));
            }
            multiindex.erase(multiindex.begin(), multiindex.end() - problem_.dim);
        }
        return sub2ind(multiindex);
    }

private:
    UniformIntDistribution distr;
    size_t stride_;
    std::vector<size_t> multiindex;
};

class HypercubeBinSampler : public HypercubeSampler {
public:
    HypercubeBinSampler(HypercubeProblem problem, size_t stride, bool littleEndian, BitsRepack &bits_repack)
        : HypercubeSampler(problem), littleEndian_(littleEndian), bits_repack_(bits_repack) {}
    virtual size_t operator()(RandomBitGenerator &rng) {
        size_t idx = 0;
        for (size_t n = 0; n < problem_.dim; n++)
            idx = (idx << 1) + pop_front(rng);
        return idx;
    }
    bool pop_front(RandomBitGenerator &rng) {
        if (bits.empty()) {
            size_t val = bits_repack_(rng);
            if (littleEndian_) {
                // start adding from hi bit
                uint64_t high_mask = 1;
                for (size_t n = 1; n < rng.nbits(); n++) {
                    high_mask <<= 1;
                }
                for (size_t n = 0; n < rng.nbits(); n++) {
                    bits.push(val % high_mask != 0);
                    val <<= 1;
                }
            } else {
                // start adding from lo bit
                for (size_t n = 0; n < rng.nbits(); n++) {
                    bits.push(val % 2 != 0);
                    val >>= 1;
                }
            }
        }
        bool res = bits.front();
        bits.pop();
        return res;
    };

private:
    std::queue<bool> bits;
    bool littleEndian_;
    BitsRepack &bits_repack_;
};

#endif