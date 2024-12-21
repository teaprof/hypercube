#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include<array>

template<class T, size_t _size>
class CircularBuffer
{
public:
    CircularBuffer(): begin(0) {}

    void shiftleft(T val)
    {
        buf[begin] = val;
        if(++begin == _size)
            begin = 0;
    }

    T operator[](int index) const
    //index should be 0...size-1 or -1, -2, ..., -size
    {
        index = index + begin;
        while(index < 0)
            index += _size;
        while(index >= _size)
            index -= _size;
        return buf[index];
    }

    constexpr static size_t size()
    {
        return _size;
    }

private:
    std::array<T, _size> buf;
    size_t begin;
};

template<class ostream, class T, size_t size>
ostream& operator<<(ostream& str, const CircularBuffer<T, size>& buf)
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
