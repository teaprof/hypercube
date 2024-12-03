#include<vector>
#include<atomic>
#include<mutex>
#include<memory>

class Histogram {
    public:
        Histogram() {};
        virtual ~Histogram() {};
        virtual void allocate(size_t size) {
            data.assign(size, 0);
        }
        virtual void increment(size_t idx) {
            data[idx]++;
        }
        virtual size_t operator[](size_t idx) const {
            return data[idx];
        }
        virtual size_t size() const {
            return data.size();
        }
    protected:
        std::vector<size_t> data;
};


class HistogramAtomic : public Histogram {
    public:
        HistogramAtomic() {};
        ~HistogramAtomic() {};        
        void increment(size_t idx) {
            std::atomic_ref<size_t> a(data[idx]);
            a++;
        }
};


class HistogramMutexed : public Histogram {
    public:
        HistogramMutexed(size_t nMutexes = 100) {
            for(size_t n = 0; n < nMutexes; n++) {
                mutexes.emplace_back(std::make_unique<std::mutex>());
            }
        };
        ~HistogramMutexed() {};        
        void increment(size_t idx) {
            std::lock_guard lk(*mutexes[idx % mutexes.size()]);
            data[idx]++;
        }
    private:
        std::vector<std::unique_ptr<std::mutex>> mutexes;
};
