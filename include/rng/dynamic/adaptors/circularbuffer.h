#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include<array>
#include<vector>
#include<cassert>

template<class T>
class CircularBuffer
{
public:
    CircularBuffer(const size_t size, T default_val = T()): buf_(size, default_val) { }

    void push(T val)
    {
        buf_[begin_] = val;
        begin_ = (begin_ + 1) % buf_.size();
        //if(++begin == _size)
        //    begin = 0;
    }
    T operator[](int index) const
    //index should be 0...size-1 or -1, -2, ..., -size
    {        
        if(index < 0) {
            assert(-index <= buf_.size());
            size_t n = (buf_.size() + begin_ + index ) % buf_.size();
            return buf_[n];
        }
        assert(index < buf_.size());
        size_t n = (index + begin_) % buf_.size();
        return buf_[n];
    }
    size_t size() const
    {
        return buf_.size();
    }

private:
    std::vector<T> buf_;
    size_t begin_{0};
    size_t end_{0};
};


template<class ostream, class T>
ostream& operator<<(ostream& str, const CircularBuffer<T>& buf)
{
    for(size_t i = 0; i < buf.size(); i++)
    {
        str<<buf[i];
        if(i+1 != buf.size())
            str<<" ";
    }
    return str;
}
#endif // CIRCULARBUFFER_H
