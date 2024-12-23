#ifndef STATISTICAL_TEST_BASE_
#define STATISTICAL_TEST_BASE_

#include "arrays/Histogram.h"
#include "rng/RandomNumberWrapper.h"

#include <map>
#include <thread>
#include <vector>
#include <iostream>

struct StatiscticalTestResults {
    size_t dof; // degrees of freedom
    double chi2;
    size_t sum, sum2;
    size_t N;
};

struct SubtaskParameters {
    size_t Ntasks;
    size_t cur_task;
    size_t n_threads;
};

struct TaskResults {
    size_t sum, sum2;
};

struct Chi2BasedProblem {
    size_t N;
    size_t m_intervals_total;
};

class DistributionSampler {
public:
    virtual ~DistributionSampler() {}
    virtual size_t operator()(RandomBitGenerator &rng) = 0;
    virtual size_t max() = 0;
    virtual void discardN(size_t N, RandomBitGenerator &rng) = 0;
};

template <class Histogram> class Chi2BasedTest {
public:
    Chi2BasedTest() {}
    virtual ~Chi2BasedTest() {}

    TaskResults run(const Chi2BasedProblem &problem, const SubtaskParameters &subtask, std::shared_ptr<DistributionSampler> sampler,
                    std::shared_ptr<RandomBitGenerator> rng) {
        assert(problem.m_intervals_total == sampler->max() + 1);
        size_t data_size = getSubarraySize(problem, subtask);
        data.allocate(data_size);
        std::vector<std::thread> threads;
        for (size_t thread_id = 0; thread_id < subtask.n_threads; thread_id++) {
            // Chi2BasedTest::runThread<RandomNumberWrapperT,
            // MultiindexGeneratorT>(task, subtask, rng, sampler, subtask.n_threads,
            // thread_id);
            // std::thread thread(&Chi2BasedTest::runThread, this, task, subtask, sampler, rng, thread_id);
            Chi2BasedTest::runThread(problem, subtask, sampler, rng, thread_id);
            // threads.emplace_back(std::move(thread));
        }
        for (auto &it : threads)
            it.join();
        TaskResults res = getResults();
        // data.clear();
        // data.shrink_to_fit();
        return res;
    }

    StatiscticalTestResults collect(const Chi2BasedProblem &task, size_t n_subtasks,
                                    const std::vector<TaskResults> &subtask_results) {
        assert(n_subtasks == subtask_results.size());
        StatiscticalTestResults res{.dof = 0, .chi2 = 0, .sum = 0, .sum2 = 0, .N = 0};
        res.dof = task.m_intervals_total - 1;
        res.N = task.N;
        res.sum = std::accumulate(subtask_results.begin(), subtask_results.end(), 0,
                                  [](auto sum, auto &it) { return sum + it.sum; });
        res.sum2 = std::accumulate(subtask_results.begin(), subtask_results.end(), 0,
                                   [](auto sum2, auto &it) { return sum2 + it.sum2; });
        res.chi2 = 0;
        return res;
    }
private:
    std::pair<size_t, size_t> split(size_t N, size_t total_tasks, size_t cur_task) {
        size_t start = N * (cur_task) / total_tasks;
        size_t end = N * (cur_task + 1) / total_tasks;
        return {start, end};
    }

    TaskResults getResults() {
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
                   std::shared_ptr<RandomBitGenerator> rng, size_t thread_id) {
        TaskResults res{.sum = 0, .sum2 = 0};
        auto [thread_start, thread_end] = split(problem.N, subtask.n_threads, thread_id);
        sampler->discardN(thread_start, *rng);
        for (size_t n = 0; n < thread_end - thread_start; n++) {
            size_t idx = (*sampler)(*rng);
            increment(subtask, idx);
        }
    }
    Histogram data;
};

// using HypercubeSampler = KIndependentGenerator;

#endif