#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include<array>
#include<vector>
#include<cassert>
#include<cstddef>

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

template<class ValueT>
class CircularQueue
{
	/* CircularQueue with limited max capacity. If capacity is exceeded, the new values will overwrite the old ones.
	 *
	 * writeIterator - the iterator pointing to the place where new element should be stored.
	 * readIterator  - the iterator pointing to the place where new element should be read from.
	 * if readIterator == writeIterator then buffer considered to be empty.
	 *
	 * The size of underlying std::vector should be equal to the required size of buffer plus 1 due to
	 * the following reason:
	 * Consider the full buffer. The element, writeIterator pointing to, contains a valid value that will
	 * be overwritten on the next call of push(). But we cannot extract it, because if readIterator == writeIterator
	 * the buffer is considered to be empty.
	 */
	public:
		CircularQueue() {};
		CircularQueue(size_t maxsize)
		{
            setCapacity(maxsize);
		}

        void setCapacity(size_t maxsize) {
            if(maxsize == this->maxsize) {
                return;
            }
			//we require one more element due to the reason described above
			this->maxsize = maxsize + 1;

			//we reserve all memory needed to avoid reallocating memory in the
			//future because this makes iterators invalid.
			buf.reserve(maxsize+1);
        }

		/* Pushes v into circular buffer.
		 * If it is possible, implementation uses ValueT::swap operation. Otherwise it uses copy
		 * constructor to build new element in the buffer.
		 * */
		void push_back(ValueT v)
		{
			assert(maxsize > 0);
			if(buf.size() < maxsize)
			{
				//adding the next element
				buf.push_back(v);
				if(buf.size() == 1)
					readIterator = buf.begin();
				if(buf.size() < maxsize)
					writeIterator = buf.end();
				else
					writeIterator = buf.begin();
			} else {
				//replace the element
				*writeIterator = v;
				//increase write iterator
				if(++writeIterator == buf.end())
					writeIterator = buf.begin();
			}
			//increase read iterator if needed
			if(writeIterator == readIterator)
			{
				readIterator++;
				if(readIterator == buf.end())
					readIterator = buf.begin();
			}
		}
		/// Remove an element from the front of the buffer
		void pop_front()
		{
			assert(!buf.empty());
			assert(readIterator != writeIterator);
			if(++readIterator == buf.end() && writeIterator != buf.end())
				readIterator = buf.begin();
		}
		/// Return front element from the buffer
        ValueT front() {
			assert(!buf.empty());
			assert(readIterator != writeIterator);
			return *readIterator;

        }
		bool empty()
		{
			if(buf.empty())
				return true;
			return readIterator == writeIterator;
		}
	private:
		std::vector<ValueT> buf;
		//typename std::vector<Byte>::contiguous_iterator writeIterator, readIterator;
		typename std::vector<ValueT>::iterator writeIterator, readIterator;
		size_t maxsize{0};
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
