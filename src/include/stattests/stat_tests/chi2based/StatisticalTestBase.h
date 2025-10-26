#ifndef STATISTICAL_TEST_BASE_
#define STATISTICAL_TEST_BASE_

#include <stattests/arrays/Histogram.h>
#include <librandom/rng/dynamic/generators/RandomBitGenerator.h>
#include <boost/math/distributions/chi_squared.hpp>

#include <map>
#include <thread>
#include <vector>
#include <iostream>
#include <cassert>

struct Chi2BasedProblem {
    size_t N;
    size_t n_cells_total;
    bool operator==(const Chi2BasedProblem& other) const {
        return N == other.N && n_cells_total == other.n_cells_total;
    }
    bool checkRNGSufficiency(const RandomBitGenerator &rng) const {
        (void)rng;
        return true;
    }
};


struct StatiscticalTestResults {
    size_t dof; // degrees of freedom
    uint64_t sum, sum2;
    size_t N; // should be equal to sum
    double chi2_precise;
    bool operator==(const StatiscticalTestResults& other) const {
        //return dof == other.dof && chi2 == other.chi2 && chi2cdf == other.chi2cdf && mean == other.mean && sum == other.sum && sum2 == other.sum2 && N == other.N;
        return dof == other.dof && sum == other.sum && sum2 == other.sum2 && N == other.N;
    }
    double mean() const {
        return static_cast<double>(N)/(dof+1);
    }
    double chi2() const {
        return sum2/mean() - sum;
    }
    double chi2cdf() const {
        boost::math::chi_squared_distribution dist(dof);
        //return boost::math::cdf(dist, chi2());
        return boost::math::cdf(dist, chi2_precise);
    }
};

struct SubtaskParameters {
    size_t Ntasks{1};
    size_t cur_task{1};
    bool operator==(const SubtaskParameters& other) const {
        return Ntasks == other.Ntasks && cur_task == other.cur_task;
    }
};

struct SubtaskResults {
    size_t dof;
    size_t sum, sum2;
    double chi2_precise;
    bool parameters_ok;
    bool operator==(const SubtaskResults& other) const {
        return sum == other.sum && sum2 == other.sum2;
    }
    double mean() const {
        return static_cast<double>(sum)/(dof+1);
    }
    double chi2() const {
        return sum2/mean() - sum;
    }
    double chi2cdf() const {
        boost::math::chi_squared_distribution dist(dof);
        //return boost::math::cdf(dist, chi2());
        return boost::math::cdf(dist, chi2_precise);
    }
};

class DistributionSampler {
public:
    virtual ~DistributionSampler() {}
    virtual std::shared_ptr<DistributionSampler> copy() = 0;
    virtual size_t operator()(RandomBitGenerator &rng) = 0; 
    virtual size_t max() const = 0;
    virtual void discardN(uint64_t N, RandomBitGenerator &rng) = 0;
    /// return number of rng calls to produce the desired number outputs
    virtual uint64_t getNumberOfRngCalls(uint64_t numberOfSampleCalls) const = 0;
    virtual double getProbability(size_t value, const RandomBitGenerator &rng) const = 0;
};

template <class Histogram> 
class Chi2BasedTest {
public:
    Chi2BasedTest() {}
    virtual ~Chi2BasedTest() {}

    template<class ProblemType>
    SubtaskResults run(const ProblemType &problem, const SubtaskParameters &subtask, DistributionSampler& sampler,
                std::shared_ptr<RandomBitGenerator> rng, size_t n_threads = 1) {                    
        static_assert(std::derived_from<ProblemType, Chi2BasedProblem>); // we cant use `requires` clause since it is not supported by SWIG
        assert(problem.n_cells_total == sampler.max() + 1);
        size_t data_size = getSubarraySize(problem, subtask);
        //std::cout<<"Allocating "<<data_size<<" cells"<<std::endl;
        data.allocate(data_size);
        std::vector<std::thread> threads;
        size_t prev_thread_start = 0;
        //std::cout<<"problem.N = "<<problem.N<<std::endl;        
        for (size_t thread_id = 0; thread_id < n_threads; thread_id++) {
            // Chi2BasedTest::runThread<RandomNumberWrapperT,
            // MultiindexGeneratorT>(problem, subtask, rng, sampler, subtask.n_threads, thread_id);
            auto [thread_start, thread_end] = split(problem.N, n_threads, thread_id);            
            sampler.discardN(thread_start - prev_thread_start, *rng);
            prev_thread_start = thread_start;
            std::cout<<"Starting thread "<<thread_id<<"\n";
            //Chi2BasedTest::runThread(problem, subtask, sampler, rng->copy(), n_threads, thread_id);
             std::thread thread(&Chi2BasedTest::runThread, this, problem, subtask, sampler.copy(), rng->copy(), n_threads, thread_id);
            threads.emplace_back(std::move(thread));
        }
        for (auto &it : threads)
            it.join();
        SubtaskResults res = getResults(problem, subtask, sampler, rng);
        res.parameters_ok = problem.checkRNGSufficiency(*rng);
        // data.clear()
        // data.shrink_to_fit();
        return res;
    }

    StatiscticalTestResults collect(const Chi2BasedProblem &problem, size_t n_subtasks,
                                    const std::vector<SubtaskResults> &subtask_results) {
        assert(n_subtasks == subtask_results.size());
        //StatiscticalTestResults res{.dof = 0, .chi2 = 0, .sum = 0, .sum2 = 0, .N = 0};
        StatiscticalTestResults res{.dof = 0, .sum = 0, .sum2 = 0, .N = 0};
        res.dof = problem.n_cells_total - 1;
        res.N = problem.N;
        res.sum = std::accumulate(subtask_results.begin(), subtask_results.end(), static_cast<uint64_t>(0),
                                  [](auto sum, auto &it) { return sum + it.sum; });
        res.sum2 = std::accumulate(subtask_results.begin(), subtask_results.end(), static_cast<uint64_t>(0),
                                   [](auto sum2, auto &it) { return sum2 + it.sum2; });
        res.chi2_precise = std::accumulate(subtask_results.begin(), subtask_results.end(), static_cast<double>(0),
                                   [](auto chi2_precise, auto &it) { return chi2_precise + it.chi2_precise; });
        //res.mean = static_cast<double>(res.N)/problem.n_cells_total;
        //res.chi2 = res.sum2/res.mean - res.N;
        res.dof = problem.n_cells_total-1;
        //boost::math::chi_squared_distribution dist(res.dof);
        //res.chi2cdf = boost::math::cdf(dist, res.chi2);
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

    SubtaskResults getResults(const Chi2BasedProblem &problem, const SubtaskParameters &subtask, DistributionSampler& sampler, std::shared_ptr<RandomBitGenerator> rng) {
        uint64_t sum = 0, sum2 = 0;
        for (size_t n = 0; n < data.size(); n++) {
            uint64_t v = data[n];
            sum += v;
            sum2 += v * v;
        }
        SubtaskResults res;
        res.dof = data.size() - 1;
        res.sum2 = sum2;
        res.sum = sum;
        res.chi2_precise = getChi2Precise(problem, subtask, sampler, rng);
        res.parameters_ok = false;
        return res;
    }
    
    double getChi2Precise(const Chi2BasedProblem &problem, const SubtaskParameters &subtask, DistributionSampler& sampler, std::shared_ptr<RandomBitGenerator> rng) {
        double chi2 = 0;
        for(size_t idx = 0; idx < problem.n_cells_total; idx++) {
            if(idx % subtask.Ntasks == subtask.cur_task) {
                double expected = sampler.getProbability(idx, *rng);
                expected*=problem.N;
                double delta = (data[idx] - expected)*(data[idx] - expected)/expected;
                chi2 += delta;
            }
        }
        return chi2;
    }
    

    size_t getSubarraySize(const Chi2BasedProblem problem, const SubtaskParameters& subtask) {
        size_t res = 0;
        for(size_t n = 0; n < problem.n_cells_total; n++)
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
        //std::cout<<thread_end - thread_start<<std::endl;
        //std::cout<<(*rng)()<<std::endl;
        /*std::cout<<(*rng)()<<std::endl;
        std::cout<<(*rng)()<<std::endl;*/
        for (size_t n = 0; n < thread_end - thread_start; n++) {
            size_t idx = (*sampler)(*rng);            
            //std::cout<<idx<<" ";
            increment(subtask, idx);
        }
        //std::cout<<"\n";
    }
    Histogram data;
};

//using Chi2BasedTest1 = Chi2BasedTest<HistogramAtomic>;

// using HypercubeSampler = KIndependentGenerator;

#endif