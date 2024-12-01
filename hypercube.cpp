#include<vector>
#include<random>
#include<cassert>
#include<cinttypes>
#include<chrono>
#include<iostream>

std::vector<uint16_t> data;

const size_t dim = 4;
const size_t m = 40;
const double aver_points_per_cell = 10;
size_t N = 0;


size_t power(size_t base, size_t pow) {
    size_t res = 1;
    for(size_t n = 0; n < pow; n++) 
        res *= base;
    return res;
}

void initialize() {
    data.assign(power(m, dim), 0);
    N = aver_points_per_cell*data.size();
}

size_t sub2ind(const std::vector<size_t>& multiindex) {
    size_t idx = 0;
    assert(multiindex.size() == dim);
    for(size_t n = 0; n < dim; n++) {
        idx = m*idx + multiindex[n];
    }
    return idx;
}

void run() {
    std::mt19937 gen;
    //size_t gen = 0;
    std::vector<size_t> multiindex(dim, 0);
    for(size_t n = 0; n < N; n++) {
        for(size_t k = 0; k < dim; k++) {
            multiindex[k] = gen() % m;
        }
        size_t idx = sub2ind(multiindex);
        assert(idx < data.size());
        data[idx]++;
    }
}

int main() {
    initialize();
    std::vector<size_t> multiindex{dim, 0};
    auto tstart = std::chrono::high_resolution_clock::now();
    run();
    auto tend = std::chrono::high_resolution_clock::now();

    auto period = std::chrono::high_resolution_clock::period();
    double dt = static_cast<double>(period.num)/period.den;
    double t = (tend - tstart).count()*dt;
    std::cout<<"t "<<t*1000<<" ms"<<std::endl;
    return 0;
}