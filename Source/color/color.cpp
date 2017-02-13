#include <algorithm>

#include "color/color.h"

namespace rvg {
    namespace color {

uint8_t unorm_to_uint8_t(float f) {
    f = std::min(255.f, std::max(0.f, f*256.f));
    return static_cast<uint8_t>(f);
}

float uint8_t_to_unorm(int i) {
    constexpr float a = 1.f/254.f;
    constexpr float b = -.5f/254.f;
    return std::min(1.f, std::max(0.f, a*i+b));
}

uint8_t int_to_uint8_t(int i) {
    return static_cast<uint8_t>(std::min(255, std::max(0, i)));
}

RGBA8 rgba(float r, float g, float b, float a) {
    return RGBA8{
        unorm_to_uint8_t(r),
        unorm_to_uint8_t(g),
        unorm_to_uint8_t(b),
        unorm_to_uint8_t(a)
    };
}

RGBA8 rgba8(int r, int g, int b, int a) {
    return RGBA8{
        int_to_uint8_t(r),
        int_to_uint8_t(g),
        int_to_uint8_t(b),
        int_to_uint8_t(a)
    };
}

RGBA8 rgb(float r, float g, float b) {
    return RGBA8{
        unorm_to_uint8_t(r),
        unorm_to_uint8_t(g),
        unorm_to_uint8_t(b),
        255
    };
}

RGBA8 rgb8(int r, int g, int b) {
    return RGBA8{
        int_to_uint8_t(r),
        int_to_uint8_t(g),
        int_to_uint8_t(b),
        255
    };
}

} } // namespace rvg::color
