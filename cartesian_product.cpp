#include <vector>
#include <tuple>
#include <optional>
#include <iostream>
#include <cassert>

template<class ... Args>
class CartesianProduct2 {
	    using types = std::tuple<typename Args::value_type...>;
	    static constexpr size_t n_subspaces = std::tuple_size_v<types>;
	    static_assert(n_subspaces > 0);
    public:
        CartesianProduct2(const Args& ... args) : subspaces_(args...) {};
        
        struct Iterator {
		        Iterator(const CartesianProduct2<Args...> &obj, size_t pos) : obj_(obj), pos_(pos) {}
	        	decltype(auto) operator*() {
	        		return obj_[pos_];
    	    	}
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

		decltype(auto) operator[](size_t combination_number) const {
			return multiIndexToSingleIndicies<0>(combination_number);
		}        
        
        template<size_t subspace_number>
        decltype(auto) multiIndexToSingleIndicies(size_t combination_number) const {
	        auto subspace = std::get<subspace_number>(subspaces_);
        	std::optional<size_t> idx;
	        if(!subspace.empty()) {
	        	idx = combination_number % subspace.size();
	        	combination_number /= subspace.size();
        	}        	
        	return std::tuple_cat(std::make_tuple(idx), multiIndexToSingleIndicies<subspace_number+1>(combination_number));
        }

        template<>
        decltype(auto) multiIndexToSingleIndicies<n_subspaces>(size_t combination_number) const {
	        auto subspace = std::get<0>(subspaces_);
        	std::optional<size_t> idx;
	        if(!subspace.empty()) {
	        	idx = combination_number % subspace.size();
	        	combination_number /= subspace.size();
        	}        	
        	return std::make_tuple(idx);
        }
        
                
        template<size_t subspace_number>
		decltype(auto) get(size_t idx) {
			using subspace_type = typename std::tuple_element<subspace_number, std::tuple<Args...>>::type;
			using value_type = typename subspace_type::value_type;
			using return_type = std::optional<value_type>;
			
			auto subspace = std::get<subspace_number>(subspaces_);
			if(subspace.empty()) {			
				return static_cast<return_type>(std::nullopt);
			};
			return static_cast<return_type>(subspace[idx]);
		}
        template<size_t subspace_number, typename ArgType>
		decltype(auto) get(std::optional<size_t> idx, ArgType default_value) {
			using subspace_type = typename std::tuple_element<subspace_number, std::tuple<Args...>>::type;
			using value_type = typename subspace_type::value_type;
			
			if(!idx.has_value()) {
				return static_cast<value_type>(default_value);
			};
			auto subspace = std::get<subspace_number>(subspaces_);
			assert(*idx < subspace.size());
			return static_cast<value_type>(subspace[*idx]);
		}
    private:
    	std::tuple<Args...> subspaces_;
};

template<class T>
std::ostream& operator<<(std::ostream& str, std::optional<T> v) {
	if(v.has_value()) {
		str<<v.value();
	} else {
		str<<"{}";
	}
	return str;
}

int main() {
	std::vector<int> a{1, 2, 3};
	std::vector<double> b{1, 2};
	std::vector<double> c{};
	CartesianProduct2 v(a, b, c);
	//std::<<v.get<0>(0).value()<<std::endl;
	//std::cout<<v.get<1>(1).value()<<std::endl;
	for(auto it = v.begin(); it != v.end(); ++it) {				
		std::optional<size_t> idx1 = std::get<0>(*it);
		std::optional<size_t> idx2 = std::get<1>(*it);
		std::optional<size_t> idx3 = std::get<2>(*it);
		int v1 = v.get<0>(*idx1).value();
		double v2 = v.get<1>(*idx2).value();
		auto v3_opt = v.get<2>(idx3, 42);
		//double v3 = v.get<2>(idx3, 42);
		std::cout<<it.pos()<<" -> ["<<idx1<<" "<<idx2<<" "<<idx3<<"] -> ["<<v1<<" "<<v2<<" "<<v3_opt<<"]"<<std::endl;
	}
	return 0;
}
