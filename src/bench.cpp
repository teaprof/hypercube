#include <rng/dynamic/generators/RandomNumberWrapper.h>
#include <rng/dynamic/adaptors/bitsrepack.h>
#include <stat_tests/hypercube/HypercubeTest.h>
#include <progress/tictoc.h>

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

static void bitsRepack(benchmark::State& state) {
    auto rng = std::make_shared<MT19937Wrapper>(); /// \todo: use fake but fast generator
    BitsRepack repacker(rng, 32, true, true);
    for(auto _: state) {
        repacker(*rng);
    }
    state.counters["rate"] = benchmark::Counter(state.iterations(), benchmark::Counter::kIsRate);        
}

static void hypercubeBenchSingleThread(benchmark::State& state) {
    const size_t Nsamples = 1'000'000;
    HypercubeProblem problem(3, 16, 16, Nsamples);
    auto sampler = std::make_shared<HypercubeSampler>(problem);
    SubtaskParameters subtask{.Ntasks=1,.cur_task=0, .n_threads=1};
    auto rng = std::make_shared<MT19937Wrapper>();
    Chi2BasedTest<Histogram> test;
        
    //tic();
    for(auto _ : state) {
        test.run(problem, subtask, sampler, rng->copy());
    }
    state.counters["rate"] = benchmark::Counter(state.iterations()*Nsamples, benchmark::Counter::kIsRate);
    //double t1 = toc();        
    //std::cout<<t1<<std::endl;

}


static void hypercubeBenchAtomic(benchmark::State& state) {
    const size_t Nsamples = 1'000'000;
    const size_t threads = 8;
    HypercubeProblem problem(3, 16, 16, Nsamples);
    auto sampler = std::make_shared<HypercubeSampler>(problem);
    SubtaskParameters subtask{.Ntasks=1,.cur_task=0, .n_threads=threads};
    auto rng = std::make_shared<MT19937Wrapper>();
    Chi2BasedTest<HistogramAtomic> test_atomic;
        
    //tic();
    for(auto _ : state) {
        test_atomic.run(problem, subtask, sampler, rng->copy());
    }
    state.counters["rate"] = benchmark::Counter(state.iterations()*Nsamples, benchmark::Counter::kIsRate);
    //double t1 = toc();        
    //std::cout<<t1<<std::endl;

}

static void hypercubeBenchMutexed(benchmark::State& state) {
    const size_t Nsamples = 1'000'000;
    const size_t threads = 8;
    HypercubeProblem problem(3, 16, 16, Nsamples);
    auto sampler = std::make_shared<HypercubeSampler>(problem);
    SubtaskParameters subtask{.Ntasks=1,.cur_task=0, .n_threads=threads};
    auto rng = std::make_shared<MT19937Wrapper>();
    Chi2BasedTest<HistogramMutexed> test_mutexed;
        
    //tic();
    for(auto _ : state) {
        test_mutexed.run(problem, subtask, sampler, rng->copy());
    }
    state.counters["rate"] = benchmark::Counter(state.iterations()*Nsamples, benchmark::Counter::kIsRate);
    //double t1 = toc();        
    //std::cout<<t1<<std::endl;

}

BENCHMARK(stdMT19937);
BENCHMARK(teaMT19937);
BENCHMARK(bitsRepack);
BENCHMARK(hypercubeBenchSingleThread);
BENCHMARK(hypercubeBenchAtomic);
BENCHMARK(hypercubeBenchMutexed);

BENCHMARK_MAIN();