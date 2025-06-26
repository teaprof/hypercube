#include <rnglib/rng/dynamic/generators/RandomNumberWrapper.h>
#include <rnglib/rng/dynamic/adaptors/bitsrepack.h>
#include <app/progress/tictoc.h>


static void bitsRepackStd(size_t N) {
    auto rng = std::make_shared<MT19937Wrapper>(); /// \todo: use fake but fast generator
    BitsRepackStd repacker(rng, 32, true, true);
    tic();
    for(size_t n = 0; n < N; n++) {
        repacker(*rng);
    }
    auto t = toc();
}

static void bitsRepackCircular(size_t N) {
    auto rng = std::make_shared<MT19937Wrapper>(); /// \todo: use fake but fast generator
    BitsRepackCircular repacker(rng, 32, true, true);
    tic();
    for(size_t n = 0; n < N; n++) {
        repacker(*rng);
    }
    auto t = toc();
}

int main() {
    size_t N = 1'000'000'0;
    bitsRepackStd(N);
    bitsRepackCircular(N);
}
