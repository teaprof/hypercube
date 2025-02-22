#include<options/cartesian_product.h>

#include<sstream>
#include<vector>
#include<tuple>

#include<gtest/gtest.h>


namespace {
template<class T>
std::ostream& operator<<(std::ostream& str, std::optional<T> v) {
	if(v.has_value()) {
		str<<v.value();
	} else {
		str<<"{}";
	}
	return str;
}
}

TEST(CARTESIANPRODUCT, SIMPLETEST) {
	std::vector<int> a{1, 2, 3};
	std::vector<double> b{10, 11};
	std::vector<double> c{};
	CartesianProduct2 v(a, b, c);
	std::optional<size_t> idx1, idx2, idx3;
	int v1;
	double v2;
	std::optional<double> v3;
	auto it = v.begin();	
	idx1 = std::get<0>(*it);
	idx2 = std::get<1>(*it);
	idx3 = std::get<2>(*it);
	ASSERT_EQ(idx1, 0);
	ASSERT_EQ(idx2, 0);
	ASSERT_FALSE(idx3.has_value());
	v1 = v.get<0>(*idx1).value();
	v2 = v.get<1>(*idx2).value();
	v3 = v.get<2>(idx3, 42);
    ASSERT_EQ(v1, 1);;
    ASSERT_EQ(v2, 10);
    ASSERT_EQ(v3, 42);
    v3 = v.get<2>(idx3);
    ASSERT_FALSE(v3.has_value());
	//std::cout<<it.pos()<<" -> ["<<idx1<<" "<<idx2<<" "<<idx3<<"] -> ["<<v1<<" "<<v2<<" "<<v3<<"]"<<std::endl;
	++it;
	++it;
	++it;
	++it;
	idx1 = std::get<0>(*it);
	idx2 = std::get<1>(*it);
	idx3 = std::get<2>(*it);
	ASSERT_EQ(idx1, 1);
	ASSERT_EQ(idx2, 1);
	ASSERT_FALSE(idx3.has_value());
	v1 = v.get<0>(*idx1).value();
	v2 = v.get<1>(*idx2).value();
	v3 = v.get<2>(idx3, 43);
    ASSERT_EQ(v1, 2);;
    ASSERT_EQ(v2, 11);
    ASSERT_EQ(v3, 43);
	//std::cout<<it.pos()<<" -> ["<<idx1<<" "<<idx2<<" "<<idx3<<"] -> ["<<v1<<" "<<v2<<" "<<v3<<"]"<<std::endl;
}
