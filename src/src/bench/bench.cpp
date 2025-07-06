#include <librandom/rng/dynamic/generators/RandomNumberWrapper.h>
#include <librandom/rng/dynamic/adaptors/bitsrepack.h>
#include <stattests/stat_tests/hypercube/HypercubeSampler.h>
#include <app/progress/tictoc.h>

#include <utility>
#include <benchmark/benchmark.h>


static void stdMT19937(benchmark::State& state) {
    std::mt19937 rng;
    for(auto _ : state) {
        rng();
    }
    state.counters["rate"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}

static void teaMT19937(benchmark::State& state) {
    MT19937Wrapper rng;
    for(auto _ : state) {
        rng();
    }
    state.counters["rate"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);
}

static void bitsRepackStd25(benchmark::State& state) {
    auto rng = std::make_shared<MT19937Wrapper>(); /// \todo: use fake but fast generator
    BitsRepackStd repacker(rng, 25, true, true);
    for(auto _: state) {
        repacker(*rng);
    }
    state.counters["rate"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);        
}

static void bitsRepackCircular25(benchmark::State& state) {
    auto rng = std::make_shared<MT19937Wrapper>(); /// \todo: use fake but fast generator
    BitsRepackCircular repacker(rng, 25, true, true);
    for(auto _: state) {
        repacker(*rng);
    }
    state.counters["rate"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);        
}

template<bool src_little_endian, bool dst_little_endian>
static void bitsRepackFast25(benchmark::State& state) {
    auto rng = std::make_shared<MT19937Wrapper>(); /// \todo: use fake but fast generator
    BitsRepackFast repacker(rng, 25,src_little_endian, dst_little_endian);
    for(auto _: state) {
        repacker(*rng);
    }
    state.counters["rate"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);        
}

template<bool src_little_endian, bool dst_little_endian>
static void bitsRepackFast42(benchmark::State& state) {
    auto rng = std::make_shared<MT19937Wrapper>(); /// \todo: use fake but fast generator
    BitsRepackFast repacker(rng, 42,src_little_endian, dst_little_endian);
    for(auto _: state) {
        repacker(*rng);
    }
    state.counters["rate"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);        
}


static void hypercubeBenchSingleThread(benchmark::State& state) {
    const size_t Nsamples = 1'000'000;
    HypercubeProblem problem(3, 16, 16, Nsamples);
    //auto sampler = std::make_shared<HypercubeSampler>(problem);
    HypercubeSampler sampler(problem);
    SubtaskParameters subtask{.Ntasks=1,.cur_task=0};
    auto rng = std::make_shared<MT19937Wrapper>();
    Chi2BasedTest<Histogram> test;
        
    //tic();
    const size_t n_threads = 1;
    for(auto _ : state) {
        test.run(problem, subtask, sampler, rng->copy(), n_threads);
    }
    state.counters["rate"] = benchmark::Counter(state.iterations()*Nsamples, benchmark::Counter::kIsRate);
    //double t1 = toc();        
    //std::cout<<t1<<std::endl;

}


static void hypercubeBenchAtomic(benchmark::State& state) {
    const size_t Nsamples = 1'000'000;
    const size_t threads = 8;
    HypercubeProblem problem(3, 16, 16, Nsamples);
    //auto sampler = std::make_shared<HypercubeSampler>(problem);
    HypercubeSampler sampler(problem);
    SubtaskParameters subtask{.Ntasks=1,.cur_task=0};
    auto rng = std::make_shared<MT19937Wrapper>();
    Chi2BasedTest<HistogramAtomic> test_atomic;
        
    //tic();
    for(auto _ : state) {
        test_atomic.run(problem, subtask, sampler, rng->copy(), threads);
    }
    state.counters["rate"] = benchmark::Counter(state.iterations()*Nsamples, benchmark::Counter::kIsRate);
    //double t1 = toc();        
    //std::cout<<t1<<std::endl;

}

static void hypercubeBenchMutexed(benchmark::State& state) {
    const size_t Nsamples = 1'000'000;
    const size_t threads = 8;
    HypercubeProblem problem(3, 16, 16, Nsamples);
    //auto sampler = std::make_shared<HypercubeSampler>(problem);
    HypercubeSampler sampler(problem);
    SubtaskParameters subtask{.Ntasks=1,.cur_task=0};
    auto rng = std::make_shared<MT19937Wrapper>();
    Chi2BasedTest<HistogramMutexed> test_mutexed;
        
    //tic();
    for(auto _ : state) {
        test_mutexed.run(problem, subtask, sampler, rng->copy(), threads);
    }
    state.counters["rate"] = benchmark::Counter(state.iterations()*Nsamples, benchmark::Counter::kIsRate);
    //double t1 = toc();        
    //std::cout<<t1<<std::endl;

}

BENCHMARK(stdMT19937);
BENCHMARK(teaMT19937);
BENCHMARK(bitsRepackStd25);
BENCHMARK(bitsRepackCircular25);
BENCHMARK_TEMPLATE(bitsRepackFast25, false, false);
BENCHMARK_TEMPLATE(bitsRepackFast25, false, true);
BENCHMARK_TEMPLATE(bitsRepackFast25, true, false);
BENCHMARK_TEMPLATE(bitsRepackFast25, true, true);
BENCHMARK_TEMPLATE(bitsRepackFast42, false, false);
BENCHMARK_TEMPLATE(bitsRepackFast42, false, true);
BENCHMARK_TEMPLATE(bitsRepackFast42, true, false);
BENCHMARK_TEMPLATE(bitsRepackFast42, true, true);
BENCHMARK(hypercubeBenchSingleThread);
BENCHMARK(hypercubeBenchAtomic);
BENCHMARK(hypercubeBenchMutexed);

BENCHMARK_MAIN();