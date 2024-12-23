#include "stat_tests/hypercube/HypercubeTest.h"

#include <gtest/gtest.h>

#include<random>
#include<vector>

struct ReferenceTest {
    size_t dim;
    size_t m_intervals_per_dim;
    size_t stride;
    size_t Nsamples;
    std::mt19937 rng;
    std::pair<uint64_t, uint64_t> operator()() {
        size_t mIntervalsTotal = 1;
        for(size_t n = 0; n < dim; n++)
            mIntervalsTotal *= m_intervals_per_dim;
        
        std::vector<uint16_t> data(mIntervalsTotal, 0);
        std::deque<size_t> values;
        for(size_t n = 0; n < Nsamples; n++) {
            if(values.empty()) {
                for(size_t n = 0; n < dim; n++)
                    values.push_back(rng() % m_intervals_per_dim);
            } else {
                for(size_t n = 0; n < stride; n++) {
                    values.push_back(rng() % m_intervals_per_dim);
                    values.pop_front();
                }
            }
            size_t idx = 0;
            for(auto v : values) {
                idx = idx*m_intervals_per_dim + v;
            }
            data[idx]++;
        }

        uint64_t sum{0}, sum2{0};
        for(auto it : data) {
            uint64_t v = it;
            sum += v;
            sum2 += v*v;
        }
        return {sum, sum2};
    }
};

class HypercubeTestSuite : public testing::TestWithParam<std::tuple<size_t, size_t, size_t, size_t, size_t>> {
    public:
        size_t dim;
        size_t mIntervals;
        size_t stride;
        size_t nSubtasks;
        size_t NPoints;

        void SetUp() override {
            dim = std::get<0>(GetParam());
            mIntervals = std::get<1>(GetParam());
            stride = std::get<2>(GetParam());
            nSubtasks = std::get<3>(GetParam());
            NPoints = std::get<4>(GetParam());
            if(stride == 0) {
                stride = dim;
            }
        }

        TaskResults expected;
        void calculateExpected() {
            ReferenceTest reference_test{.dim=dim,.m_intervals_per_dim=mIntervals,.stride=stride,.Nsamples=NPoints};
            auto expected_cur = reference_test();
            expected = {expected_cur.first, expected_cur.second};
        }

        TaskResults received;
        void calcuateReceived() {
            HypercubeProblem problem{dim, mIntervals, stride, NPoints};
            received = {0, 0};
            for(size_t n = 0; n < nSubtasks; n++) {
                auto sampler = std::make_shared<HypercubeSampler>(problem);
                auto rng = std::make_shared<MT19937Wrapper>();
                SubtaskParameters subtask{.Ntasks=nSubtasks,.cur_task=n,.n_threads=1};
                Chi2BasedTest<Histogram> test;
                auto cur_received = test.run(problem, subtask, sampler, rng);
                received.sum += cur_received.sum;
                received.sum2 += cur_received.sum2;
            }
        }
};

TEST_P(HypercubeTestSuite, BaseTest) { 
    calculateExpected();
    calcuateReceived();
    EXPECT_EQ(received.sum, expected.sum);
    EXPECT_EQ(received.sum2, expected.sum2);
}


INSTANTIATE_TEST_SUITE_P(HypercubeTest0, HypercubeTestSuite, testing::Combine(
    testing::Values(1, 3), // dim
    testing::Values(2, 32, 17, 100), // mIntervalsPerDim
    testing::Values(0, 1, 2, 17), // stride
    testing::Values(1, 2, 5), // nSubtasks
    testing::Values(1, 10, 100, 1000, 100000)), // NPoints
    [](const testing::TestParamInfo<HypercubeTestSuite::ParamType>& info)
    {
        const size_t dim = std::get<0>(info.param);
        const size_t mIntervals = std::get<1>(info.param);
        const size_t stride = std::get<2>(info.param);
        const size_t nSubtasks = std::get<3>(info.param);
        const size_t NPoints = std::get<4>(info.param);
        std::stringstream res;
        res<<"dim"<<dim<<"_mIntervals"<<mIntervals<<"_stride"<<stride<<"_nSubtask"<<nSubtasks<<"_NPoints"<<NPoints;
        return res.str();
    });
