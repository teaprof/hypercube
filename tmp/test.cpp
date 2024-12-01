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
size_t N = 1000000;


size_t sub2ind1(const std::vector<size_t>& multiindex) {
    size_t idx = 0;
    for(size_t n = 0; n < N; n++)
	    for(size_t n = 0; n < dim; n++) {
	        idx = m*idx + multiindex[n];
	    }
    return idx;
}

size_t sub2ind2(const std::vector<size_t>& multiindex) {
    size_t idx = 0;
    for(size_t n = 0; n < N; n++)
	    for(size_t n = 0; n < multiindex.size(); n++) {
    	    idx = m*idx + multiindex[n];
    	}
    return idx;
}

size_t sub2ind3(const std::vector<size_t>& multiindex) {
    size_t idx = 0;
    for(size_t n = 0; n < N; n++)    
	    for(const auto it: multiindex) {
	        idx = m*idx + it;
	    }
    return idx;
}


int main() {
    std::vector<size_t> multiindex{dim, 0};    
    auto tstart = std::chrono::high_resolution_clock::now();
    sub2ind1(multiindex);
    auto tend1 = std::chrono::high_resolution_clock::now();
    sub2ind2(multiindex);
    auto tend2 = std::chrono::high_resolution_clock::now();
    sub2ind3(multiindex);
    auto tend3 = std::chrono::high_resolution_clock::now();
	
    auto period = std::chrono::high_resolution_clock::period();
    double dt = static_cast<double>(period.num)/period.den;
    double t1 = (tend1 - tstart).count()*dt;
    double t2 = (tend2 - tend1).count()*dt;
    double t3 = (tend3 - tend2).count()*dt;
    std::cout<<"t1 "<<t1*1000<<" ms"<<std::endl;
    std::cout<<"t2 "<<t2*1000<<" ms"<<std::endl;
    std::cout<<"t3 "<<t2*1000<<" ms"<<std::endl;
    return 0;
}
