#ifndef __STAT_TESTS_RNG_H_
#define __STAT_TESTS_RNG_H_

#include<rng/dynamic/generators/RandomBitGenerator.h>
#include<rng/dynamic/generators/RandomNumberWrapper.h>
#include<rng/dynamic/adaptors/bitsrepack.h>
#include<memory>
#include<cstddef>
#include<optional>

struct RandomNumberGeneratorDescription {
    size_t rng_id;
    std::optional<size_t> seed; // if not set the default value for selected generator is used
    uint64_t offset;
    bool operator==(const RandomNumberGeneratorDescription& other) const {
        return rng_id == other.rng_id && seed == other.seed && offset == other.offset;
    }
};

struct BitsRepackDescription {
    bool src_little_endian;
    bool dst_little_endian;
    std::optional<uint16_t> bits_per_sample;
    bool operator==(const BitsRepackDescription& other) const {
        return src_little_endian == other.src_little_endian && dst_little_endian == other.dst_little_endian && bits_per_sample == other.bits_per_sample;
    }
};

std::shared_ptr<RandomBitGenerator> createGenerator(const RandomNumberGeneratorDescription& rng_descr, std::optional<BitsRepackDescription> bits_repack_descr) {
    auto rng = std::make_shared<MT19937Wrapper>();
    if(rng_descr.seed) {
        rng->rng.seed(*rng_descr.seed);
    }
    rng->discardN(rng_descr.offset);
    if(bits_repack_descr) {
        //Use the default value for bits_per_sample for selected rng
        uint16_t bits_per_sample = rng->nbits();
        if(bits_repack_descr->bits_per_sample) {
            //if bits_repack_descr.bits_per_sample is set, use this value
            bits_per_sample = *bits_repack_descr->bits_per_sample;
        }
        auto repacker = std::make_shared<BitsRepackFast>(rng, bits_per_sample, bits_repack_descr->src_little_endian, bits_repack_descr->dst_little_endian);
        return repacker;
    }
    return rng;
}

#endif