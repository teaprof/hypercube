#ifndef __META_DATA_H__
#define __META_DATA_H__

#include <cstdint>
#include <optional>
#include <vector>
#include <hash/hash.h>

struct MetaData {
    uint64_t taskId;
    hash_t hash;

    bool operator==(const MetaData& other) const {
        return taskId == other.taskId && hash == other.hash;
    }
};

#endif