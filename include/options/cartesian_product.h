#ifndef __CARTESIAN_PRODUCT_H
#define __CARTESIAN_PRODUCT_H

#include<vector>
#include<optional>

class CartesianProduct {
    public:
        void addDimension(size_t nElements) {
            space_dimensions.push_back(nElements);
            if(nElements > 0) {
                product_space_size *= nElements;
            }
        }
        std::vector<std::optional<size_t>> indicies(size_t index) const {
            std::vector<std::optional<size_t>> res;
            res.reserve(space_dimensions.size());
            for(auto cur_space_dim : space_dimensions) {
                if(cur_space_dim == 0) {
                    res.push_back(std::nullopt);
                } else {
                    res.push_back(index % cur_space_dim);
                    index /= cur_space_dim;
                }
            }
            return res;
        }
        template<class T>
        static T select(const std::vector<T>& values, std::optional<size_t> index, const T& default_v) {
            if(!index) {
                return default_v;
            };
            return values[*index];
        }
        template<class T>
        static std::optional<T> select(const std::vector<T>& values, std::optional<size_t> index) {
            if(!index) {
                return std::nullopt;
            };
            return values[*index];
        }
        size_t productSpaceSize() const {
            return product_space_size;
        }
    private:
        std::vector<size_t> space_dimensions;    
        size_t product_space_size{1};
};

template<class T, class ... Args>
class CartesianProduct2 {
    public:
        CartesianProduct2(const T& val, const Args& ... args) : val_(val), tail_(args...) {}
        CartesianProduct2() {}
    private:
        const T& val_;
        CartesianProduct2<Args...> tail_;
};

#endif