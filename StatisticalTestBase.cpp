#include "StatisticalTestBase.h"
#include "progress/tictoc.h"

#include <cassert>
#include <numeric>

HypercubeTestParameters::HypercubeTestParameters(size_t dim, size_t m_intervals_per_dim, size_t N) {
    this->dim = dim;
    this->m_intervals_per_dim = m_intervals_per_dim;
    this->N = N;
    m_intervals_total = 1;
    for(size_t k = 0; k < dim; k++)
        m_intervals_total *= m_intervals_per_dim;
}

