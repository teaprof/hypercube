#ifndef __CARTESIAN_PRODUCT_H
#define __CARTESIAN_PRODUCT_H

#include<vector>
#include<optional>
#include<tuple>
#include<cassert>

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

template<class ... Args>
class CartesianProduct2 {
	    using types = std::tuple<typename Args::value_type...>;
        
        
        using subspaces_t = std::tuple<Args...>;
	    static constexpr size_t n_subspaces = std::tuple_size_v<types>;        
	    static_assert(n_subspaces > 0);

        template<typename From, typename To>
        using replace_fn = std::conditional_t<false, From, To>;
    public:
        using index_t = std::tuple<replace_fn<Args, std::optional<size_t>>...>;

        CartesianProduct2() = default;
        CartesianProduct2(const Args& ... args) : subspaces_(args...) {};
        
        template<size_t subspace_number, class T>
        void setSubspace(const std::vector<T>& v) {
            std::get<subspace_number>(subspaces_) = v;
        }

        struct Iterator {
		        Iterator(const CartesianProduct2<Args...> &obj, size_t pos) : obj_(obj), pos_(pos) {}
	        	index_t operator*() const {
	        		return obj_[pos_];
    	    	}
                using value_type = index_t;
	        	void operator++() {
    	    		pos_++;
    	    	}
    	    	bool operator==(const Iterator& other) const {
    	    		return &(this->obj_) == &obj_ && pos_ == other.pos_;
    	    	}
    	    	size_t pos() const {
    	    		return pos_;
    	    	}
        	private:
	        	const CartesianProduct2<Args...>& obj_;
	        	size_t pos_;
        };
        
        Iterator begin() const {
        	return Iterator(*this, 0);
        }
        Iterator end() const {
        	return Iterator(*this, spaceSize());
        }
        
        size_t spaceSize() const {
        	return spaceSizeImpl<n_subspaces>();
        }
        
        
		index_t operator[](size_t combination_number) const {
			return multiIndexToSingleIndiciesImpl<0>(combination_number);
		}        
        
                
        template<size_t subspace_number>
		decltype(auto) get(std::optional<size_t> idx) const {
			using subspace_type = typename std::tuple_element<subspace_number, std::tuple<Args...>>::type;
			using value_type = typename subspace_type::value_type;
			using return_type = std::optional<value_type>;
			
			auto subspace = std::get<subspace_number>(subspaces_);
			if(!idx.has_value() || subspace.empty()) {			
				return static_cast<return_type>(std::nullopt);
			};
			return static_cast<return_type>(subspace[*idx]);
		}
        template<size_t subspace_number, typename ArgType>
		decltype(auto) get(std::optional<size_t> idx, ArgType default_value) const {
			using subspace_type = typename std::tuple_element<subspace_number, std::tuple<Args...>>::type;
			using value_type = typename subspace_type::value_type;
			
			if(!idx.has_value()) {
				return static_cast<value_type>(default_value);
			};
			auto subspace = std::get<subspace_number>(subspaces_);
			assert(*idx < subspace.size());
			return static_cast<value_type>(subspace[*idx]);
		}
        template<size_t subspace_number>
		decltype(auto) get(const index_t& multi_index) const {
            std::optional<size_t> idx = std::get<subspace_number>(multi_index);
            return get<subspace_number>(idx);
		}
        template<size_t subspace_number, typename ArgType>
		decltype(auto) get(const index_t& multi_index, ArgType default_value) const {
            std::optional<size_t> idx = std::get<subspace_number>(multi_index);
            return get<subspace_number>(idx, default_value);
		}
    private:
    	subspaces_t subspaces_;
		template<size_t subspace_number>
        size_t spaceSizeImpl() const {
        	static_assert(subspace_number > 0);
			auto subspace = std::get<subspace_number - 1>(subspaces_);
			size_t subspace_size = subspace.size();
			if(subspace_size == 0) {
				subspace_size++;
			}
			size_t res = subspace_size*(this->template spaceSizeImpl<subspace_number-1>());
        	return res;
        }
        
		template<>
        size_t spaceSizeImpl<0>() const {
        	return 1;
        }
        template<size_t subspace_number>
        decltype(auto) multiIndexToSingleIndiciesImpl(size_t combination_number) const {
	        auto subspace = std::get<subspace_number>(subspaces_);
        	std::optional<size_t> idx;
	        if(!subspace.empty()) {
	        	idx = combination_number % subspace.size();
	        	combination_number /= subspace.size();
        	}        	
        	return std::tuple_cat(std::make_tuple(idx), multiIndexToSingleIndiciesImpl<subspace_number+1>(combination_number));
        }

        template<>
        decltype(auto)  multiIndexToSingleIndiciesImpl<n_subspaces-1>(size_t combination_number) const {
            static_assert(n_subspaces > 0);
	        auto subspace = std::get<n_subspaces-1>(subspaces_);
        	std::optional<size_t> idx;
	        if(!subspace.empty()) {
	        	idx = combination_number % subspace.size();
	        	combination_number /= subspace.size();
        	}        	
        	return std::make_tuple(idx);
        }
};

template<class BaseIterator, class MapFunction>
struct IteratorWithMapping : public BaseIterator {
    IteratorWithMapping(const BaseIterator& base_iterator, const MapFunction& fcn) : BaseIterator(base_iterator), fcn_(fcn) {}
    using base_iterator_value_type = typename BaseIterator::value_type;

    std::invoke_result_t<MapFunction, base_iterator_value_type> operator*() const {
        auto arg = BaseIterator::operator*();
        return fcn_(arg);
    }
    private:
        MapFunction fcn_;
};

#endif