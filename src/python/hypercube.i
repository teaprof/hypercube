/* File : pyhypercube.i */
%module pyhypercube

%include <std_shared_ptr.i>
%shared_ptr(RandomBitGenerator)

/*class RandomBitGenerator {
};*/

%{
#include "Hypercube.h"
void run(const Chi2BasedProblem &problem, const SubtaskParameters &subtask, DistributionSampler& sampler) {}
%}

%include <stdint.i>
%include <stl.i>

namespace std {
    %template(DVector) vector<double>;
    %template(FVector) vector<float>;
    %template(UInt8Vector) vector<uint8_t>;
}

%apply std::vector<uint8_t> &OUTPUT {std::vector<uint8_t>& restored_codeword};
%apply int& OUTPUT {int& actualnumiters};
%apply std::vector<uint8_t> &OUTPUT {std::vector<uint8_t>& finalparitychecks};
%apply std::vector<uint8_t> &INPUT {const std::vector<uint8_t>& msg};


%include "rng/dynamic/generators/RandomBitGenerator.h"

%include "stat_tests/rng.h"
%include "stat_tests/chi2based/StatisticalTestBase.h"
%include "stat_tests/hypercube/HypercubeProblem.h"
%include "stat_tests/hypercube/HypercubeSampler.h"

%template(Chi2BasedTest1) Chi2BasedTest<HistogramAtomic>;

//SubtaskResults run(const Chi2BasedProblem &problem, const SubtaskParameters &subtask, std::shared_ptr<DistributionSampler> sampler, std::shared_ptr<RandomBitGenerator> rng, size_t n_threads = 1) {

void run(const Chi2BasedProblem &problem, const SubtaskParameters &subtask, DistributionSampler& sampler);

