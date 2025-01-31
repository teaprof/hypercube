#ifndef _OPTIONS_HASH_H_
#define _OPTIONS_HASH_H_

#include<functional>
#include<boost/crc.hpp>
#include<stat_tests/chi2based/StatisticalTestBase.h>
#include<stat_tests/hypercube/HypercubeTest.h>

typedef boost::crc_optimal<64, 0x42F0E1EBA9EA3693, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, true, true> crc_64_type;
typedef crc_64_type::value_type hash_t;

template<class S> requires std::is_trivially_copyable_v<S>
hash_t myhash(crc_64_type &crc, const S& val) {
    crc.process_bytes(&val, sizeof(S));
    return crc.checksum();
}

template<class S> requires std::is_trivially_copyable_v<S>
hash_t myhash(const S& val) {
    crc_64_type crc;
    return myhash(crc, val);
}

template<class S, class ... Args> requires (std::is_trivially_copyable_v<S> && (...  && std::is_trivially_copyable_v<Args>) )
hash_t myhash(crc_64_type &crc, const S& val, const Args& ... args) {
    crc.process_bytes(&val, sizeof(S));
    myhash(crc, args...);
    return myhash(crc, args...);
}

template<class ... Args> requires (... && std::is_trivially_copyable_v<Args>)
hash_t myhash(const Args& ... args) {
    crc_64_type crc;
    return myhash(crc, args...);
}

#endif //  _OPTIONS_HASH_H_