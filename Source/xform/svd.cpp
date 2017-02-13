#include <cmath>

#include "math/math.h"
#include "point/point.h"

#include "xform/svd.h"

#include <iomanip>

namespace rvg {
    namespace xform {
// build an elementary projector from one of the
// vectors in the nullspace of a symmetric matrix
// {{r, s}, {s, t}}, which is known to be rank defficient
static Rotation projector(float r, float s, float t) {
    using rvg::math::is_almost_zero;
    if (std::abs(r) > std::abs(t)) {
        float h = std::hypot(r,s);
        if (!is_almost_zero(h)) {
            float inv_h = 1.f/h;
            return Rotation{s*inv_h,-r*inv_h};
        } else {
            return Rotation{1.f,0.f};
        }
    } else {
        float h = std::hypot(t,s);
        if (!is_almost_zero(h)) {
            float inv_h = 1.f/h;
            return Rotation{t*inv_h,-s*inv_h};
        } else {
            return Rotation{1.f,0.f};
        }
    }
}

void svd(const Linear &A, Rotation &U, Scaling &S) {
    using rvg::math::is_almost_zero;
    auto sq = [](float a) { return a*a; };
    // we start computing the two roots of the characteristic
    // polynomial of AAt as t^2 -m t + p == 0
    const float a = A[0][0], b = A[0][1], c = A[1][0], d = A[1][1];
    const float a2 = sq(a), b2 = sq(b), c2 = sq(c), d2 = sq(d);
    const float m = a2+b2+c2+d2;
    const float p = sq(b*c-a*d);
    // sqrt of discriminant
    const float D = std::hypot(b+c,a-d)*std::hypot(b-c,a+d);
    if (!is_almost_zero(m)) {
        // get two roots
        const float el0 = .5f*(m+D);
        const float el1 = p/el0;
        // so now we have the largest singular value
        const float s0 = std::sqrt(el0);
        // get projector from AAt - el0*I
        U = projector(a2+b2-el0, a*c+b*d, c2+d2-el0);
        // we will also use the smallest singular value
        const float s1 = std::sqrt(el1);
        if (!is_almost_zero(s1)) { // both singular values are above threshold
            S = Scaling{s0, s1};
        } else { // only largest is above threshold
            S = Scaling{s0, 0.f};
        }
    } else {
        // both roots are zero and so is the resulting matrix
        U = Identity{};
        S = Scaling{0.f,0.f};
    }
}

// analytic singular value decomposition of 2D matrix
void svd(const Linear &A, Rotation &U, Scaling &S, Linear &V) {
    using rvg::math::is_almost_zero;
    auto sq = [](float a) { return a*a; };
    // we start computing the two roots of the characteristic
    // polynomial of AAt as t^2 -m t + p == 0
    const float a = A[0][0], b = A[0][1], c = A[1][0], d = A[1][1];
    const float a2 = sq(a), b2 = sq(b), c2 = sq(c), d2 = sq(d);
    const float m = a2+b2+c2+d2;
    const float p = sq(b*c-a*d);
    // sqrt of discriminant
    const float D = std::hypot(b+c,a-d)*std::hypot(b-c,a+d);
    if (!is_almost_zero(m)) {
        // get two roots
        const float el0 = .5f*(m+D);
        const float el1 = p/el0;
        // so now we have the largest singular value
        const float s0 = std::sqrt(el0);
        // get projector from AAt - el0*I
        U = projector(a2+b2-el0, a*c+b*d, c2+d2-el0);
        // we will also use the smallest singular value
        const float s1 = std::sqrt(el1);
        if (!is_almost_zero(s1)) { // both singular values are above threshold
            S = Scaling{s0, s1};
            V = transpose(A)*U*Scaling{1.f/s0, 1.f/s1};
        } else { // only largest is above threshold
            using rvg::point::R2;
            S = Scaling{s0, 0.f};
            R2 v0 = (1.f/s0)*transpose(A)*R2{U.cos(), U.sin()};
            V = Linear{v0.x(), -v0.y(), v0.y(), v0.x()};
        }
    } else {
        // both roots are zero and so is the resulting matrix
        U = Identity{};
        S = Scaling{0.f,0.f};
        V = Identity{};
    }
}

} } // rvg::xform
