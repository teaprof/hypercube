#include<rnglib/rng/dynamic/adaptors/circularbuffer.h>
#include<gtest/gtest.h>

TEST(CircularBufferTest, Simple)
{
    const size_t dim = 18;
    CircularBuffer<int> buf(dim);
    for(size_t n = 0; n < dim; n++) {
        buf.push(n);
    }
    //std::cout<<buf<<std::endl;
    for(size_t n = 0; n < dim; n++) {
        ASSERT_EQ(buf[n], n);
    }
    for(size_t n = 0; n < dim; n++) {
        ASSERT_EQ(buf[-n - 1], dim - n - 1);
    }
    for(size_t k = 0; k < dim; k++) {
        buf.push(k + dim);
        //std::cout<<buf<<std::endl;
        for(size_t n = 0; n < dim; n++) {
            ASSERT_EQ(buf[n], n+k+1);
        }
    }
}
