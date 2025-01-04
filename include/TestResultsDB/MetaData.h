#ifndef __META_DATA_H__
#define __META_DATA_H__

#include <cstdint>
#include <optional>
#include <vector>

struct MetaData {
    uint64_t taskId;
    std::optional<uint64_t> parentId;
    std::vector<uint64_t> childIds;
};

#endif