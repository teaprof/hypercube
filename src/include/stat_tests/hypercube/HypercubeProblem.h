#ifndef __HYPERCUBE_PROBLEM_H__
#define __HYPERCUBE_PROBLEM_H__

#include <stat_tests/chi2based/StatisticalTestBase.h>

class HypercubeProblem : public Chi2BasedProblem {
public:
    HypercubeProblem() : dim{1}, m_intervals_per_dim{10}, stride{10000} 
    {
        N = 1000;
        n_cells_total = 1;
        for (size_t k = 0; k < dim; k++)
            n_cells_total *= m_intervals_per_dim;
    };
    HypercubeProblem(size_t _dim, size_t _m_intervals_per_dim, size_t _stride, size_t _N) : Chi2BasedProblem{.N=_N, .n_cells_total=0}, 
        dim(_dim), m_intervals_per_dim(_m_intervals_per_dim), stride(_stride) {
        n_cells_total = 1;
        for (size_t k = 0; k < dim; k++)
            n_cells_total *= m_intervals_per_dim;
    }
    bool operator==(const HypercubeProblem& other) const {
        return dim == other.dim && m_intervals_per_dim == other.m_intervals_per_dim && stride == other.stride && 
        static_cast<Chi2BasedProblem>(*this) == other;
    }
    size_t dim;
    size_t m_intervals_per_dim;
    size_t stride;
};

#endif