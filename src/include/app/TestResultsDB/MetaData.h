#ifndef __META_DATA_H__
#define __META_DATA_H__

#include <cstdint>
#include <optional>
#include <vector>
#include <app/hash/hash.h>

struct MetaData {
    uint64_t taskId;

    bool operator==(const MetaData& other) const {
        return taskId == other.taskId;
    }
};

#endif