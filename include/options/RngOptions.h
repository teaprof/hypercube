#ifndef __RNG_OPTIONS_H__
#define __RNG_OPTIONS_H__

#include <options/BasicOptions.h>
#include <options/cartesian_product.h>
#include <rng/dynamic/adaptors/bitsrepack.h>
#include <rng/dynamic/generators/RandomNumberWrapper.h>
#include <stat_tests/rng.h>
#include <boost/optional.hpp>
//#include <boost/endian.hpp>
#include <cstdint>

template<class T>
std::optional<T> to_std_optional(boost::optional<T> v) {
    if(v) {
        return *v;
    }
    return std::nullopt;
}

class RNGOptions : public OptionsGroupStorage {
    public:
        RNGOptions() : OptionsGroupStorage("RNG options") {
            namespace po = boost::program_options;
            addPartialVisible("name", po::value<std::string>(&name)->default_value("mt19937"), "rng name");
            //addPartialVisible("number", po::value<uint16_t>(&number)->default_value(0), "rng number");
            addPartialVisible("seed", po::value(&seed), "seed (if not set the default value for selected generator is used)");
            addPartialVisible("offset", po::value<uint64_t>(&offset)->default_value(0), "how many samples should be skipped");
        }            
        void validate() override {
            if(name != "mt19937") {
                throw std::range_error("unknown rng name");
            }
        }
        void update(const boost::program_options::variables_map& map) override {
            //nothing to do
        }
        RandomNumberGeneratorDescription description() {
            return {number, to_std_optional(seed), offset};
        }
        std::string name;
        size_t number{0};
        boost::optional<size_t> seed;
        size_t offset{0};
};

class BitsRepackOptions : public OptionsGroupStorage {
    public:
        BitsRepackOptions() : OptionsGroupStorage("Bits repack options") {
            namespace po = boost::program_options;
            addPartialVisible("srcLittleEndian", po::value(&src_little_endian), "is source little endian? (default is true)");
            addPartialVisible("dstLittleEndian", po::value(&dst_little_endian), "is dest little endian? (default is true)");
            addPartialVisible("dstSampleBits", po::value(&dst_sample_bits), "dst bits per sample (default - use selected RNG native sample size)");
        }
        void update(const boost::program_options::variables_map& vm) override {
            options_combinations.addDimension(src_little_endian.size());
            options_combinations.addDimension(dst_little_endian.size());
            options_combinations.addDimension(dst_sample_bits.size());
        }                
        size_t nCombinations() const {
            return options_combinations.productSpaceSize();
        }
        std::optional<BitsRepackDescription> description(size_t n) const {
            if(useRepack()) {
                auto indicies = options_combinations.indicies(n);            
                assert(indicies.size() == 3);
                bool src_little_endian_val = CartesianProduct::select(src_little_endian, indicies[0], true); /// \todo: use native endianness
                bool dst_little_endian_val = CartesianProduct::select(dst_little_endian, indicies[1], true); /// \todo: use native endianness
                std::optional<uint16_t> dst_sample_bits_val = CartesianProduct::select(dst_sample_bits, indicies[2]);
                return BitsRepackDescription{src_little_endian_val, dst_little_endian_val, dst_sample_bits_val};
            }   
            return std::nullopt;
        }
        bool useRepack() const {
            return !src_little_endian.empty() || !dst_little_endian.empty() || !dst_sample_bits.empty();
        }
        struct Iterator {            
            Iterator(const BitsRepackOptions& obj, size_t current_combination = 0) : obj_(obj), current_combination_{current_combination} {}
            std::optional<BitsRepackDescription> operator*() {
                return obj_.description(current_combination_);
            }
            void operator++() {
                current_combination_++;
            }
            bool operator==(const Iterator& other) const {
                return current_combination_ == other.current_combination_;
            }
            private:
            size_t current_combination_;
            const BitsRepackOptions& obj_;
        };
        Iterator begin() const {
            return Iterator(*this);
        }
        Iterator end() const {
            return Iterator(*this, options_combinations.productSpaceSize());
        }
        std::vector<bool> src_little_endian, dst_little_endian;
        std::vector<uint16_t> dst_sample_bits;
        CartesianProduct options_combinations;
};


#endif