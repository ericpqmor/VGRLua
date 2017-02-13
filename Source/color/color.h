#ifndef RVG_PAINT_COLOR_H
#define RVG_PAINT_COLOR_H

#include <cstdint>

namespace rvg {
    namespace color {

uint8_t unorm_to_uint8_t(float f);
float uint8_t_to_unorm(int i);
uint8_t int_to_uint8_t(int i);

class RGBA8 {
    uint8_t m_r, m_g, m_b, m_a;
public:
    RGBA8(void): m_r(0), m_g(0), m_b(0), m_a(0) { ; }

    RGBA8(int r, int g, int b, int a = 255):
        m_r(int_to_uint8_t(r)), m_g(int_to_uint8_t(g)),
        m_b(int_to_uint8_t(b)), m_a(int_to_uint8_t(a)) { ; }

    RGBA8(float r, float g, float b, float a = 1.0f):
        m_r(unorm_to_uint8_t(r)), m_g(unorm_to_uint8_t(g)),
        m_b(unorm_to_uint8_t(b)), m_a(unorm_to_uint8_t(a)) { ; }

    // so colors can be inserted into unordered maps
    bool operator==(const RGBA8 &o) const {
        return m_r == o.m_r && m_g == o.m_g && m_b == o.m_b && m_a == o.m_a;
    }

    uint8_t r(void) const { return m_r; }
    uint8_t g(void) const { return m_g; }
    uint8_t b(void) const { return m_b; }
    uint8_t a(void) const { return m_a; }
};

RGBA8 rgba(float r, float g, float b, float a);
RGBA8 rgba8(int r, int g, int b, int a);
RGBA8 rgb(float r, float g, float b);
RGBA8 rgb8(int r, int g, int b);

} } // namespace rvg::color

#include <functional>

namespace std {

// so colors can be inserted in unordered maps
template <> struct hash<rvg::color::RGBA8> {
    size_t operator()(const rvg::color::RGBA8 &c) const {
        return hash<int32_t>()(c.r() + (c.g() << 8) + (c.b() << 16)
            + (c.a() << 24));
    }
};

}

#endif

