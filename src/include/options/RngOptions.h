#ifndef __RNG_OPTIONS_H__
#define __RNG_OPTIONS_H__

#include <rng/dynamic/adaptors/bitsrepack.h>
#include <rng/dynamic/generators/RandomNumberWrapper.h>
#include <stat_tests/rng.h>
#include <options/cartesian_product.h>

#include <ProgramOptionsHeavy.h>

#include <functional>
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

class RNGOptions : public program_options_heavy::OptionsGroup {
    public:

        RNGOptions() : OptionsGroup("RNG options") {
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
        std::string name; // rng name
        size_t number{0}; // rng number (i.e. uniq ID)
        boost::optional<size_t> seed;
        size_t offset{0};
};

class BitsRepackOptions : public program_options_heavy::OptionsGroup {
        std::vector<bool> src_little_endian, dst_little_endian;
        std::vector<uint16_t> dst_sample_bits;
    public:        
        using cartesian_product_t = CartesianProduct2<std::vector<bool>, std::vector<bool>, std::vector<uint16_t>>;        
        BitsRepackOptions() : OptionsGroup("Bits repack options") {
            namespace po = boost::program_options;
            addPartialVisible("srcLittleEndian", po::value(&src_little_endian), "is source little endian? (default is true)");
            addPartialVisible("dstLittleEndian", po::value(&dst_little_endian), "is dest little endian? (default is true)");
            addPartialVisible("dstSampleBits", po::value(&dst_sample_bits), "dst bits per sample (default - use selected RNG native sample size)");
        }
        void update(const boost::program_options::variables_map& vm) override {
            options_combinations_.setSubspace<0>(src_little_endian);
            options_combinations_.setSubspace<1>(dst_little_endian);
            options_combinations_.setSubspace<2>(dst_sample_bits);
        }                
        size_t nCombinations() const {
            return options_combinations_.spaceSize();
        }
        std::optional<BitsRepackDescription> description(const cartesian_product_t::index_t &multi_index) const {
            if(useRepack()) {
                bool src_little_endian_val = options_combinations_.get<0>(multi_index, true); /// \todo: use native endianness
                bool dst_little_endian_val = options_combinations_.get<1>(multi_index, true); /// \todo: use native endianness
                std::optional<uint16_t> dst_sample_bits_val = options_combinations_.get<2>(multi_index);
                return BitsRepackDescription{src_little_endian_val, dst_little_endian_val, dst_sample_bits_val};
            }   
            return std::nullopt;
        }
        bool useRepack() const {
            return !src_little_endian.empty() || !dst_little_endian.empty() || !dst_sample_bits.empty();
        }

        decltype(auto) begin() const {
            auto f = std::bind(&BitsRepackOptions::description, this, std::placeholders::_1);
            return IteratorWithMapping(options_combinations_.begin(), f);
        }
        decltype(auto) end() const {
            auto f = std::bind(&BitsRepackOptions::description, this, std::placeholders::_1);
            return IteratorWithMapping(options_combinations_.end(), f);
        }
    private:
        cartesian_product_t options_combinations_;        
};

#endif
