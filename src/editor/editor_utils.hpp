#pragma once

#include "std.hpp"

// Bezpieczne parsowanie liczb z stringów (używane przez importer i preview)
inline int safeParseInt(const std::string& value, int defaultVal) {
    if (value.empty()) return defaultVal;
    try {
        return std::stoi(value);
    } catch (...) {
        return defaultVal;
    }
}
