#ifndef __RNG_OPTIONS_H__
#define __RNG_OPTIONS_H__

#include <options/BasicOptions.h>
#include <rng/RandomNumberWrapper.h>
#include <cstdint>

class RNGOptions : public PartialOptions {
    public:
        RNGOptions() : PartialOptions("RNG options") {
            namespace po = boost::program_options;
            addPartialVisible("name", po::value<std::string>(&name)->default_value("mt19937"), "rng name");
            //addPartialVisible("number", po::value<uint16_t>(&number)->default_value(0), "rng number");
            addPartialVisible("seed", po::value<uint64_t>(&seed)->default_value(32), "seed");
            addPartialVisible("offset", po::value<uint64_t>(&offset)->default_value(true), "how many samples should be skipped");
        }            
        void validate() override {
            if(name != "mt19937") {
                throw std::range_error("unknown rng name");
            }
        }
        void update(const boost::program_options::variables_map& map) override {
            //nothing to do
        }
        std::string name;
        size_t number;
        size_t seed;
        size_t offset;
};

class BitsRepackOptions : public PartialOptions {
    public:
        BitsRepackOptions() : PartialOptions("Bits repack options") {
            namespace po = boost::program_options;
            addPartialVisible("repack", po::value<bool>(&use_repack)->default_value(false), "set to true to repack rng samples");
            addPartialVisible("srcLittleEndian", po::value<bool>(&src_little_endian)->default_value(true), "is source little endian?");
            addPartialVisible("srcSampleBits", po::value<uint16_t>(&src_sample_bits)->default_value(0), "src bits per sample, 0 for natural");
            addPartialVisible("dstLittleEndian", po::value<bool>(&dst_little_endian)->default_value(true), "is dest little endian?");
            addPartialVisible("dstSampleBits", po::value<uint16_t>(&dst_sample_bits)->default_value(32), "dst bits per sample");
        }
        void update(const boost::program_options::variables_map& vm) override {
            //nothing to do            
        }
        bool use_repack{false};
        bool src_little_endian, dst_little_endian;
        uint16_t src_sample_bits, dst_sample_bits;
};

std::shared_ptr<RandomBitGenerator> createGenerator(const RNGOptions rng_opts, const BitsRepackOptions& repack_opts) {
    auto rng = std::make_shared<MT19937Wrapper>();
    if(repack_opts.use_repack) {
        auto res = std::make_shared<BitsRepack>(rng, repack_opts.src_sample_bits, repack_opts.dst_sample_bits, repack_opts.src_little_endian, repack_opts.dst_little_endian);
        return res;
    }
    return rng;
}


#endif