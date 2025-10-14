/* File : pyhypercube.i */
%module pyhypercube

%include <std_shared_ptr.i>
%shared_ptr(RandomBitGenerator)
%shared_ptr(RandomNumberWrapperStd<std::mt19937>)

/*class RandomBitGenerator {
};*/

%{
#include "pyhypercube.h"
void run(const Chi2BasedProblem &problem, const SubtaskParameters &subtask, DistributionSampler& sampler) {}
%}

%include <stdint.i>
%include <stl.i>

namespace std {
    %template(DVector) vector<double>;
    %template(FVector) vector<float>;
    %template(UInt8Vector) vector<uint8_t>;
    %template(SubtaskResultsVector) vector<SubtaskResults>;    


%typemap(in) std::optional<uint16_t> {
    if ($input == Py_None) {
        $1 = std::nullopt;
    } else {
        $1 = PyInt_AsLong($input);
    }
}
%typemap(out) std::optional<uint16_t> {
    if($1.has_value()) {
        $result = PyLong_FromLong($1.value());
    } else {
        $result = Py_None;
    }
}
}

/*%apply std::vector<uint8_t> &OUTPUT {std::vector<uint8_t>& restored_codeword};
%apply int& OUTPUT {int& actualnumiters};
%apply std::vector<uint8_t> &OUTPUT {std::vector<uint8_t>& finalparitychecks};
%apply std::vector<uint8_t> &INPUT {const std::vector<uint8_t>& msg};*/


%include "librandom/rng/dynamic/generators/RandomBitGenerator.h"
%include "librandom/rng/dynamic/generators/RandomNumberWrapper.h"

%include "stattests/stat_tests/rng.h"
%include "stattests/stat_tests/chi2based/StatisticalTestBase.h"
%include "stattests/stat_tests/hypercube/HypercubeProblem.h"
%include "stattests/stat_tests/hypercube/HypercubeSampler.h"


%template(run) Chi2BasedTest::run<HypercubeProblem>;
%template(Chi2BasedTest1) Chi2BasedTest<HistogramAtomic>;
%template(Chi2BasedTestSingleThreaded1) Chi2BasedTest<Histogram>;

//SubtaskResults run(const Chi2BasedProblem &problem, const SubtaskParameters &subtask, std::shared_ptr<DistributionSampler> sampler, std::shared_ptr<RandomBitGenerator> rng, size_t n_threads = 1) {

%template(MT19937Wrapper) RandomNumberWrapperStd<std::mt19937>;


void run(const Chi2BasedProblem &problem, const SubtaskParameters &subtask, DistributionSampler& sampler);

