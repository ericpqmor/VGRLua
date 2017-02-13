#ifndef RVG_MATH_H
#define RVG_MATH_H

#include <cmath>
#include <limits>
#include <tuple>

namespace rvg {
    namespace math {

constexpr float rad(float deg) {
    return deg*3.14159265358979f/180.f;
}

constexpr float deg(float rad) {
    return rad*180.f/3.14159265358979f;
}

constexpr double rad(double deg) {
    return deg*3.141592653589793238463/180.;
}

constexpr double deg(double rad) {
    return rad*180./3.141592653589793238463;
}

template <typename F>
constexpr F maxrel(void) {
    return static_cast<F>(8) * std::numeric_limits<F>::epsilon();
}

template <typename F>
constexpr F maxabs(void) {
    return static_cast<F>(8) * std::numeric_limits<F>::epsilon();
}

template <typename F>
bool is_almost_equal(F a, F b, F mr = maxrel<F>(), F ma = maxabs<F>()) {
    F diff = std::abs(a-b);
    if (diff < ma) return true;
    a = std::abs(a);
    b = std::abs(b);
    F largest = (b > a) ? b : a;
    return diff <= largest * mr;
}

template <typename F>
bool is_almost_one(F f, F mr = maxrel<F>(), F ma = maxabs<F>()) {
    F diff = std::abs(f-static_cast<F>(1));
    if (diff < ma) return true;
    f = std::abs(f);
    F largest = (f > static_cast<F>(1)) ? f : static_cast<F>(1);
    return diff <= largest * mr;
}

template <typename F>
bool is_almost_zero(F f, F ma = maxabs<F>()) {
    return std::abs(f) < ma;
}

std::tuple<float, float> curvature(float ddx, float ddy, float ddw,
    float dx, float dy, float dw, float x, float y, float w);

inline float len(float dx, float dy) {
    return std::sqrt(dx*dx+dy*dy);
}

// Compute choose(n,k).
//
// We use the relation choose(n,k) = (choose(n, k-1) * (n-k+1))/k
// to recursively compute choose(n,0), choose(n,1), ..., and finally
// choose(n,k) using min(n-k,k) muls and divs.
// Since the multiplication is evaluated before the
// division, no floating-point numbers are needed. Since the
// division happens right after the product, no overflow
// happens.  This function is marked constexpr because it is
// evaluated by the compiler when the compiler knows n and
// k. In this way, many other algorithms that depend on
// choose(n,k) can be implemented faster.
//
constexpr int choose(int n, int k) {
    return ((2*k > n)?
        choose(n, n-k):
        ((k > n || k < 0)?
            0:
            ((k == n || k == 0)?
                1:
                ((n-k+1)*(choose(n, k-1)))/k
            )
        )
    );
}

} } // namespace rvg::math

#endif
