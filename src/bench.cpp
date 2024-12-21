#include <rng/RandomNumberWrapper.h>

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
BENCHMARK(stdMT19937);
BENCHMARK(teaMT19937);

BENCHMARK_MAIN();