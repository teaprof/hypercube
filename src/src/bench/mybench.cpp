#include <librandom/rng/dynamic/generators/RandomNumberWrapper.h>
#include <librandom/rng/dynamic/adaptors/bitsrepack.h>
#include <app/progress/tictoc.h>

#include <iostream>


static void bitsRepackStd(size_t N) {
    auto rng = std::make_shared<MT19937Wrapper>(); /// \todo: use fake but fast generator
    BitsRepackStd repacker(rng, 32, true, true);
    tic();
    for(size_t n = 0; n < N; n++) {
        repacker(*rng);
    }
    auto t = toc();
    std::cout<<"bitsRepackStd: "<<t<<" secs"<<std::endl;
}

static void bitsRepackCircular(size_t N) {
    auto rng = std::make_shared<MT19937Wrapper>(); /// \todo: use fake but fast generator
    BitsRepackCircular repacker(rng, 32, true, true);
    tic();
    for(size_t n = 0; n < N; n++) {
        repacker(*rng);
    }
    auto t = toc();
    std::cout<<"bitsRepackCircular: "<<t<<" secs"<<std::endl;
}

int main() {
    size_t N = 1'000'000'0;
    bitsRepackStd(N);
    bitsRepackCircular(N);
    return 0;
}
