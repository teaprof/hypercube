#ifndef STATISTICAL_TEST_BASE_
#define STATISTICAL_TEST_BASE_

#include <arrays/Histogram.h>
#include <rng/dynamic/generators/RandomBitGenerator.h>
#include <boost/math/distributions/chi_squared.hpp>

#include <map>
#include <thread>
#include <vector>
#include <iostream>
#include <cassert>

struct Chi2BasedProblem {
    size_t N;
    size_t m_intervals_total;
    bool operator==(const Chi2BasedProblem& other) const {
        return N == other.N && m_intervals_total == other.m_intervals_total;
    }
};

struct StatiscticalTestResults {
    size_t dof; // degrees of freedom
    double chi2, chi2cdf, mean;
    size_t sum, sum2;
    size_t N;
    bool operator==(const StatiscticalTestResults& other) const {
        return dof == other.dof && chi2 == other.chi2 && chi2cdf == other.chi2cdf && mean == other.mean && sum == other.sum && sum2 == other.sum2 && N == other.N;
    }
};

struct SubtaskParameters {
    size_t Ntasks;
    size_t cur_task;
    bool operator==(const SubtaskParameters& other) const {
        return Ntasks == other.Ntasks && cur_task == other.cur_task;
    }
};

struct SubtaskResults {
    size_t sum, sum2;
    bool operator==(const SubtaskResults& other) const {
        return sum == other.sum && sum2 == other.sum2;
    }
};

class DistributionSampler {
public:
    virtual ~DistributionSampler() {}
    virtual std::shared_ptr<DistributionSampler> copy() = 0;
    virtual size_t operator()(RandomBitGenerator &rng) = 0;
    virtual size_t max() = 0;
    virtual void discardN(size_t N, RandomBitGenerator &rng) = 0;
};

template <class Histogram> 
class Chi2BasedTest {
public:
    Chi2BasedTest() {}
    virtual ~Chi2BasedTest() {}

    SubtaskResults run(const Chi2BasedProblem &problem, const SubtaskParameters &subtask, std::shared_ptr<DistributionSampler> sampler,
                    std::shared_ptr<RandomBitGenerator> rng, size_t n_threads = 1) {
        assert(problem.m_intervals_total == sampler->max() + 1);
        size_t data_size = getSubarraySize(problem, subtask);
        data.allocate(data_size);
        std::vector<std::thread> threads;
        size_t prev_thread_start = 0;
        for (size_t thread_id = 0; thread_id < n_threads; thread_id++) {
            // Chi2BasedTest::runThread<RandomNumberWrapperT,
            // MultiindexGeneratorT>(task, subtask, rng, sampler, subtask.n_threads,
            // thread_id);
            auto [thread_start, thread_end] = split(problem.N, n_threads, thread_id);
            sampler->discardN(thread_start - prev_thread_start, *rng);
            prev_thread_start = thread_start;
            //Chi2BasedTest::runThread(problem, subtask, sampler, rng->copy(), n_threads, thread_id);
             std::thread thread(&Chi2BasedTest::runThread, this, problem, subtask, sampler->copy(), rng->copy(), n_threads, thread_id);
            threads.emplace_back(std::move(thread));
        }
        for (auto &it : threads)
            it.join();
        SubtaskResults res = getResults();
        // data.clear();
        // data.shrink_to_fit();
        return res;
    }

    StatiscticalTestResults collect(const Chi2BasedProblem &task, size_t n_subtasks,
                                    const std::vector<SubtaskResults> &subtask_results) {
        assert(n_subtasks == subtask_results.size());
        StatiscticalTestResults res{.dof = 0, .chi2 = 0, .sum = 0, .sum2 = 0, .N = 0};
        res.dof = task.m_intervals_total - 1;
        res.N = task.N;
        res.sum = std::accumulate(subtask_results.begin(), subtask_results.end(), static_cast<uint64_t>(0),
                                  [](auto sum, auto &it) { return sum + it.sum; });
        res.sum2 = std::accumulate(subtask_results.begin(), subtask_results.end(), static_cast<uint64_t>(0),
                                   [](auto sum2, auto &it) { return sum2 + it.sum2; });
        res.mean = static_cast<double>(res.N)/task.m_intervals_total;
        res.chi2 = res.sum2/res.mean - res.N;
        res.dof = task.m_intervals_total-1;
        boost::math::chi_squared_distribution dist(res.dof);
        res.chi2cdf = boost::math::cdf(dist, res.chi2);
        return res;
    }
private:
    void rewind(const Chi2BasedProblem& problem, size_t n_threads, size_t thread_id, std::shared_ptr<DistributionSampler> sampler, std::shared_ptr<RandomBitGenerator> rng) {
        auto [thread_start, thread_end] = split(problem.N, n_threads, thread_id);
        sampler->discardN(thread_start, *rng);
    }
    std::pair<size_t, size_t> split(size_t N, size_t total_tasks, size_t cur_task) {
        size_t start = N * (cur_task) / total_tasks;
        size_t end = N * (cur_task + 1) / total_tasks;
        return {start, end};
    }

    SubtaskResults getResults() {
        uint64_t sum = 0, sum2 = 0;
        for (size_t n = 0; n < data.size(); n++) {
            uint64_t v = data[n];
            sum += v;
            sum2 += v * v;
        }
        return {sum, sum2};
    }

    size_t getSubarraySize(const Chi2BasedProblem problem, const SubtaskParameters& subtask) {
        size_t res = 0;
        for(size_t n = 0; n < problem.m_intervals_total; n++)
            if(n % subtask.Ntasks == subtask.cur_task)
                res++;
        return res;
    }

    void increment(const SubtaskParameters& subtask, size_t idx) {
        if(idx % subtask.Ntasks == subtask.cur_task) {
            size_t idx2 = idx/subtask.Ntasks;
            assert(idx2 < data.size());
            data.increment(idx2);
        }
    }

    void runThread(const Chi2BasedProblem &problem, const SubtaskParameters &subtask, std::shared_ptr<DistributionSampler> sampler,
                   std::shared_ptr<RandomBitGenerator> rng, size_t n_threads, size_t thread_id) {
        SubtaskResults res{.sum = 0, .sum2 = 0};
        auto [thread_start, thread_end] = split(problem.N, n_threads, thread_id);
        //sampler->discardN(thread_start, *rng);
        for (size_t n = 0; n < thread_end - thread_start; n++) {
            size_t idx = (*sampler)(*rng);            
            increment(subtask, idx);
        }
    }
    Histogram data;
};

// using HypercubeSampler = KIndependentGenerator;

#endif