#pragma once
#include <string>
#include <vector>
#include <set>
#include <cstdint>

namespace qpudsl {

enum class dType{
    INT32,
    FLOAT32,
    TemplateT
};

// Describes a slice of one tensor dimension.
//   start         – first element index 
//   size          – number of elements in the slice 
//   full_dim_size – total size of the original dimension
struct SliceRange {
    int64_t start         = 0;
    int64_t size          = -1;
    int64_t full_dim_size = -1;
};

struct Level {
    std::string index;
    // Needed for std::set
    bool operator<(const Level &other) const {
        return index < other.index;
    }

    // Needed for std::set_difference
    bool operator==(const Level &other) const {
        return index == other.index;
    }
};

using OrderMap = std::unordered_map<std::string, size_t>;
OrderMap index_order_map(const std::vector<Level> &levels);

struct Format {
    std::vector<Level> levels;
    std::set<Level> bc_levels;

     // Factories
    static Format ordered(std::vector<Level> lvls) {
        return Format{.levels = std::move(lvls)};
    }
    static Format unordered(std::set<Level> lvls) {
        return Format{.bc_levels = std::move(lvls)};
    }

    std::set<Level> get_all_levels() const {
        if (levels.empty()) {
            return bc_levels;
        } else {
            std::set<Level> copy = bc_levels;
            copy.insert(levels.cbegin(), levels.cend());
            return copy;
        }
    }
    int get_level_order(const std::string &idx) const;
    bool level_exists(const std::string &idx) const;
    bool is_bc_lvl(const std::string &idx) const;
};

struct TensorType {
    Format format;
    dType dtype;

    TensorType(Format format, dType dtype)
        : format(std::move(format)), dtype(std::move(dtype)) {}

     TensorType() : dtype(dType::TemplateT) {}
};

Format make_format(const Format &a, const Format &b);
} // namespace qpudsl
