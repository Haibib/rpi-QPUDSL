#include "Printer.h"



namespace qpudsl
{   

    std::ostream &operator<<(std::ostream &os, const Level &lvl) {
    os << lvl.index;
    return os;
}

std::ostream &operator<<(std::ostream &os, const Format &format) {
    os << "Format{";

    // Ordered levels come first
    if (!format.levels.empty()) {
        os << "[";
        for (size_t i = 0; i < format.levels.size(); ++i) {
            os << format.levels[i];
            if (i + 1 < format.levels.size())
                os << ", ";
        }
        os << "]";
    }

    // Unordered levels
    if (!format.bc_levels.empty()) {
        if (!format.levels.empty())
            os << ", "; // separator only if needed

        os << "{";
        bool first = true;
        for (const auto &lvl : format.bc_levels) {
            if (!first)
                os << ", ";
            os << lvl;
            first = false;
        }
        os << "}";
    }

    // If both empty, show explicitly
    if (format.levels.empty() && format.bc_levels.empty()) {
        os << "empty";
    }

    os << "}";

    return os;
    }
    
} // namespace qpudsl
