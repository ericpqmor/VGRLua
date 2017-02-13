#include "math/math.h"

namespace rvg {
    namespace math {

inline float det(float a, float b, float c,
          float d, float e, float f,
          float g, float h, float i) {
    return a*e*i + b*f*g  + d*h*c - c*e*g - a*f*h - b*d*i;
}

inline float sq(float x) {
    return x*x;
}

inline float cube(float x) {
    return x*x*x;
}

inline float cr(float x, float y, float u, float v) {
    return v*x - u*y;
}

std::tuple<float, float> curvature(float ddx, float ddy, float ddw,
    float dx, float dy, float dw, float x, float y, float w) {
    float s = cube(w) * det(ddx, ddy, ddw, dx, dy, dw, x, y, w);
    float t = cube(std::sqrt(sq(cr(x, w, dx, dw))+sq(cr(y, w, dy, dw))));
    return std::tuple<float, float>(s, t);
}

} } // namespace rvg::math
