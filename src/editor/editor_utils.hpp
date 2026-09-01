#pragma once

#include "std.hpp"

// Safe integer parsing from strings (used by importer and preview)
inline int safeParseInt(const std::string& value, int defaultVal) {
    if (value.empty()) return defaultVal;
    try {
        return std::stoi(value);
    } catch (...) {
        return defaultVal;
    }
}
