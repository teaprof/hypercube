#ifndef __RNG_OPTIONS_H__
#define __RNG_OPTIONS_H__

#include <options/BasicOptions.h>
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

class RNGOptions : public PartialOptions {
    public:
        RNGOptions() : PartialOptions("RNG options") {
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

class BitsRepackOptions : public PartialOptions {
    public:
        BitsRepackOptions() : PartialOptions("Bits repack options") {
            namespace po = boost::program_options;
            addPartialVisible("srcLittleEndian", po::value(&src_little_endian), "is source little endian?");
            addPartialVisible("dstLittleEndian", po::value(&dst_little_endian), "is dest little endian?");
            addPartialVisible("dstSampleBits", po::value(&dst_sample_bits), "dst bits per sample");
        }
        void update(const boost::program_options::variables_map& vm) override {
            //nothing to do            
        }
        std::optional<BitsRepackDescription> description() const {
            if(useRepack()) {
                bool src_little_endian_val = true; /// \todo: use native endianness
                bool dst_little_endian_val = true; /// \todo: use native endianness
                bool dst_sample_bits_val = 32;
                if(src_little_endian) {
                    src_little_endian_val = *src_little_endian;
                }
                if(!dst_little_endian) {
                    src_little_endian_val = *dst_little_endian;
                }                
                if(!dst_sample_bits) {
                    dst_sample_bits_val = *dst_sample_bits;
                }
                return BitsRepackDescription{src_little_endian_val, dst_little_endian_val, dst_sample_bits_val};
            }   
            return std::nullopt;         
        }
        bool useRepack() const {
            return src_little_endian || dst_little_endian || dst_sample_bits;
        }
        boost::optional<bool> src_little_endian, dst_little_endian;
        boost::optional<uint16_t> dst_sample_bits;
};


#endif