#include "StatisticalTestBase.h"

#include <cassert>
#include <numeric>

Chi2BasedTest::Chi2BasedTest() {}
Chi2BasedTest::~Chi2BasedTest() {}

std::pair<size_t, size_t> Chi2BasedTest::split(size_t N, size_t total_tasks, size_t cur_task) {
    size_t start = N*(cur_task)/total_tasks;
    size_t end = N*(cur_task+1)/total_tasks;
    return {start, end};
}


TaskResults Chi2BasedTest::getResults() {
    size_t sum = 0, sum2 = 0;
    for(auto it: data) {
        sum += it;
        sum += it*it;

    }
    return {sum, sum2};
}


StatiscticalTestResults Chi2BasedTest::collect(const Chi2BasedProblem &task, size_t n_subtasks, const std::vector<TaskResults>& subtask_results) {
    assert(n_subtasks == subtask_results.size());
    StatiscticalTestResults res{.dof=0,.chi2=0,.sum=0,.sum2=0,.N=0};
    res.dof = task.m_intervals_total - 1;
    res.N = task.N;
    res.sum = std::accumulate(subtask_results.begin(), subtask_results.end(), 0, [](auto sum, auto& it) { return sum + it.sum;});
    res.sum2 = std::accumulate(subtask_results.begin(), subtask_results.end(), 0, [](auto sum2, auto& it) { return sum2 + it.sum2;});
    res.chi2 = 0;
    return res;
}


HypercubeTestParameters::HypercubeTestParameters(size_t dim, size_t m_intervals_per_dim, size_t N) {
    this->dim = dim;
    this->m_intervals_per_dim = m_intervals_per_dim;
    this->N = N;
    m_intervals_total = 1;
    for(size_t k = 0; k < dim; k++)
        m_intervals_total *= m_intervals_per_dim;
}

