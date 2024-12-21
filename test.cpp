#include "HypercubeTest.h"

#include <gtest/gtest.h>

#include<random>
#include<vector>

struct ReferenceTest {
    size_t dim;
    size_t m_intervals_per_dim;
    size_t Nsamples;
    std::mt19937 rng;
    std::pair<uint64_t, uint64_t> operator()() {
        size_t mIntervalsTotal = 1;
        for(size_t n = 0; n < dim; n++)
            mIntervalsTotal *= m_intervals_per_dim;
        
        std::vector<uint16_t> data(mIntervalsTotal, 0);
        for(size_t n = 0; n < Nsamples; n++) {
            size_t idx = 0;
            for(size_t k = 0; k < dim; k++) {
                idx = idx*m_intervals_per_dim + rng() % m_intervals_per_dim;
            }
            data[idx]++;
        }

        uint64_t sum{0}, sum2{0};
        for(auto it : data) {
            sum += it;
            sum2 += it*it;
        }
        return {sum, sum2};
    }
};

class HypercubeTestSuite : public testing::TestWithParam<int> {


};

TEST_P(HypercubeTestSuite, BaseTest) { 
    /*const size_t dim = GetParam();   
    HypercubeProblem problem{2, 10, 2, 10000};
    auto sampler = std::make_shared<HypercubeSampler>(problem);
    auto rng = std::make_shared<MT19937Wrapper>();
    SubtaskParameters subtask{.Ntasks=1,.cur_task=0,.n_threads=1};
    Chi2BasedTest<Histogram> test;
    auto received = test.run(problem, subtask, sampler, rng);

    ReferenceTest reference_test{.dim=problem.dim,.m_intervals_per_dim=problem.m_intervals_per_dim,.Nsamples=problem.N};
    auto expected = reference_test();
    EXPECT_EQ(received.sum, expected.first);
    EXPECT_EQ(received.sum2, expected.second);*/
    std::cout << "Example Test Param: " << GetParam() << std::endl;
}

INSTANTIATE_TEST_SUITE_P(HypercubeTest0, HypercubeTestSuite, testing::Range(0, 10));