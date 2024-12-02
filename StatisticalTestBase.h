#ifndef STATISTICAL_TEST_BASE_
#define STATISTICAL_TEST_BASE_

#include "RandomNumberWrapper.h"

#include <map>
#include <vector>
#include <thread>


struct StatiscticalTestResults {
    size_t dof; //degrees of freedom    
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


class Chi2BasedTest {
public:
    Chi2BasedTest();
    virtual ~Chi2BasedTest();

    template<class RandomNumberWrapperT, class MultiindexGeneratorT>
    TaskResults run(const Chi2BasedProblem &task, const SubtaskParameters& subtask, RandomNumberWrapperT& rng, MultiindexGeneratorT& sampler) {
        TaskResults res{.sum=0,.sum2=0};
        data.assign(task.m_intervals_total, 0);
        std::vector<std::thread> threads;
        for(size_t thread_id = 0; thread_id < subtask.n_threads; thread_id++) {
            //Chi2BasedTest::runThread<RandomNumberWrapperT, MultiindexGeneratorT>(task, subtask, rng, sampler, subtask.n_threads, thread_id);
            std::thread thread(&Chi2BasedTest::runThread<RandomNumberWrapperT, MultiindexGeneratorT>, this, task, subtask, rng, sampler, subtask.n_threads, thread_id);
            threads.emplace_back(std::move(thread));
        }
        for(auto &it : threads)
            it.join();        
        res = getResults();
        data.clear();
        data.shrink_to_fit();    
        return res;
    }

    StatiscticalTestResults collect(const Chi2BasedProblem& task,  size_t n_subtasks, const std::vector<TaskResults>& subtask_results);
private:
    template<class RandomNumberWrapperT, class MultiindexGeneratorT>
    void runThread(const Chi2BasedProblem &task, const SubtaskParameters& subtask, RandomNumberWrapperT rng, MultiindexGeneratorT sampler, size_t nthreads, size_t thread_id) {
        TaskResults res{.sum=0,.sum2=0};
        auto [start, end] = split(task.N, subtask.Ntasks, subtask.cur_task);
        auto [thread_start, thread_end] = split(end - start - 1, nthreads, thread_id);
        rng.discardN(start + thread_start);
        for(size_t n = 0; n < thread_end - thread_start - 1; n++) {
            size_t idx = sampler(rng);
            data[idx]++;
        }
    }
    std::vector<size_t> data;
    TaskResults getResults();
    static std::pair<size_t, size_t> split(size_t N, size_t total_tasks, size_t cur_task);
};


class HypercubeTestParameters : public Chi2BasedProblem {
public:
    HypercubeTestParameters(size_t dim, size_t m_intervals_per_dim, size_t N);
    size_t dim;
    size_t m_intervals_per_dim;
};


using HypercubeSampler = KIndependentGenerator;

#endif