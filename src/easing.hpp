#pragma once

#include <cmath>

namespace Easing {
    inline float linear(float t) { return t; }
    inline float easeInQuad(float t) { return t * t; }
    inline float easeOutQuad(float t) { float f = 1 - t; return 1 - f * f; }
    inline float easeInOutQuad(float t) { return t < 0.5F ? 2 * t * t : 1 - (std::powf((-2 * t) + 2, 2) / 2); }
} // namespace Easing