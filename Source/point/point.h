#ifndef RVG_POINT_H
#define RVG_POINT_H

#include <iostream>
#include <tuple>
#include <array>
#include <cmath>

#include "math/math.h"
#include "point/ipoint.h"

namespace rvg {
    namespace point {

using RP1Array = std::array<float, 2>;
using RP1Tuple = std::tuple<float, float>;
using R2Array = std::array<float, 2>;
using RP2Array = std::array<float, 3>;
using R3Array = RP2Array;
using R2Tuple = std::tuple<float, float>;
using RP2Tuple = std::tuple<float, float, float>;
using R3Tuple = RP2Tuple;

class RP1 final: public IPoint<RP1> {

    RP1Array m_v;

public:

    RP1(void): m_v{{0.f, 1.f}} {
#ifdef POINT_DEBUG
        std::cerr << "RP1(void)\n";
#endif
    }

    explicit RP1(float f): m_v{{f, 1.f}} {
#ifdef POINT_DEBUG
        std::cerr << "RP1(float)\n";
#endif
    }

    RP1(float x, float w): m_v{{x, w}} {
#ifdef POINT_DEBUG
        std::cerr << "RP1(void)\n";
#endif
    }

    // conversion from array and tuple
    explicit RP1(const RP1Array &v): m_v(v) {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP1(const RP1Array &)\n";
#endif
    }

    explicit RP1(const RP1Tuple &v): m_v{{std::get<0>(v), std::get<1>(v)}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP1(const RP1Tuple &)\n";
#endif
    }

    // conversion to array and tuple
    explicit operator RP1Array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP1::operator RP1Array()\n";
#endif
        return m_v;
    }

    explicit operator RP1Tuple() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP1::operator RP1Tuple()\n";
#endif
        return RP1Tuple{m_v[0], m_v[1]};
    }

    float x(void) const { return m_v[0]; }
    float w(void) const { return m_v[1]; }

private:

    friend IPoint<RP1>;

    float do_component(int i) const { return m_v[i]; }

    // note the difference between Euclidean and projective sum
    RP1 do_added(const RP1 &o) const {
        if (m_v[1] == o.m_v[1]) {
            return RP1{ m_v[0]+o.m_v[0], m_v[1] };
        } else {
            return RP1{ m_v[0]*o.m_v[1]+o.m_v[0]*m_v[1], m_v[1]*o.m_v[1] };
        }
    }

    // note the difference between Euclidean and projective subtraction
    RP1 do_subtracted(const RP1 &o) const {
        if (m_v[1] == o.m_v[1]) {
            return RP1{ m_v[0]-o.m_v[0], m_v[1] };
        } else {
            return RP1{ m_v[0]*o.m_v[1]-o.m_v[0]*m_v[1], m_v[1]*o.m_v[1] };
        }
    }

    // note the difference between Euclidean and projective
    // scalar multiplication
    RP1 do_scaled(float s) const {
        return RP1{ s*m_v[0], m_v[1] };
    }

    // note the difference between Euclidean and projective unary minus
    RP1 do_negated(void) const {
        return RP1{ -m_v[0], m_v[1] };
    }

    bool do_is_almost_ideal(void) const {
        return std::abs(m_v[1]) < std::abs(m_v[0])*rvg::math::maxrel<float>();
    }

    bool do_is_equal(const RP1 &o) const {
        if (m_v[1] != 0.f || o.m_v[1] != 0.f) {
            return m_v[0]*o.m_v[1] == o.m_v[0]*m_v[1];
        } else {
            return true;
        }
    }

    bool do_is_almost_equal(const RP1 &o) const {
        if (!is_almost_ideal() || !o.is_almost_ideal()) {
            return rvg::math::is_almost_equal(m_v[0]*o.m_v[1], o.m_v[0]*m_v[1]);
        } else {
            return true;
        }
    }

    RP1Tuple do_untie(void) const {
        return RP1Tuple{m_v[0], m_v[1]};
    }

    std::ostream &do_print(std::ostream &out) const {
        out << "rp1{" << m_v[0] << ',' << m_v[1] << '}';
        return out;
    }

};

// Euclidean point in R2
class R2 final: public IPoint<R2> {

    R2Array m_v;

public:

    R2(void): m_v{{0.f, 0.f}} {
#ifdef POINT_DEBUG
        std::cerr << "R2(void)\n";
#endif
    }

    R2(float x, float y): m_v{{x, y}} {
#ifdef POINT_DEBUG
        std::cerr << "R2(float, float)\n";
#endif
    }

    // conversion from array and tuple
    explicit R2(const R2Array &v): m_v(v) {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2(const R2Array &)\n";
#endif
    }

    explicit R2(const R2Tuple &v): m_v{{std::get<0>(v), std::get<1>(v)}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2(const R2Tuple &)\n";
#endif
    }

    // conversion to array and tuple
    explicit operator R2Array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2::operator R2Array()\n";
#endif
        return m_v;
    }

    explicit operator R2Tuple() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2::operator R2Tuple()\n";
#endif
        return R2Tuple{m_v[0], m_v[1]};
    }

    explicit operator RP2Array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2::operator RP2Array()\n";
#endif
        return RP2Array{{m_v[0], m_v[1], 1.f}};
    }

    explicit operator RP2Tuple() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2::operator RP2Tuple()\n";
#endif
        return RP2Tuple{m_v[0], m_v[1], 1.f};
    }

#if 0
    // ??D same as RP2Array?
	explicit operator R3Array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R2::operator R3Array()\n";
#endif
        return R3Array{{m_v[0], m_v[1], 1.f}};
    }
#endif

    float x(void) const { return m_v[0]; }
    float y(void) const { return m_v[1]; }

private:
    friend IPoint<R2>;

    float do_component(int i) const { return m_v[i]; }

    R2 do_added(const R2 &o) const {
        return R2{ m_v[0]+o.m_v[0], m_v[1]+o.m_v[1] };
    }

    R2 do_subtracted(const R2 &o) const {
        return R2{ m_v[0]-o.m_v[0], m_v[1]-o.m_v[1] };
    }

    R2 do_negated() const {
        return R2{ -m_v[0], -m_v[1] };
    }

    R2 do_scaled(float s) const {
        return R2{ s*m_v[0], s*m_v[1] };
    }

    R2Tuple do_untie(void) const {
        return R2Tuple{m_v[0], m_v[1]};
    }

    bool do_is_equal(const R2 &o) const {
        return m_v[0] == o.m_v[0] && m_v[1] == o.m_v[1];
    }

    bool do_is_almost_ideal(void) const {
        return false;
    }

    bool do_is_almost_equal(const R2 &o) const {
        return rvg::math::is_almost_equal(m_v[0], o.m_v[0]) &&
            rvg::math::is_almost_equal(m_v[1], o.m_v[1]);
    }

    std::ostream &do_print(std::ostream &out) const {
        out << "r2{" << m_v[0] << ',' << m_v[1] << '}';
        return out;
    }
};

// Projective point in RP2
class RP2 final: public IPoint<RP2> {

    RP2Array m_v;

public:

    RP2(void): m_v{{0.f, 0.f, 1.f}} {
#ifdef POINT_DEBUG
        std::cerr << "RP2(void)\n";
#endif
    }

    RP2(float x, float y, float w): m_v{{x, y, w}} {
#ifdef POINT_DEBUG
        std::cerr << "RP2(float, float, float)\n";
#endif
    }

    // convertion from array and tuple
    explicit RP2(const RP2Array &v): m_v(v) {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2(const RP2Array &)\n";
#endif
    }

    explicit RP2(const RP2Tuple &v): m_v{{std::get<0>(v), std::get<1>(v), std::get<2>(v)}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2(const RP2Tuple &)\n";
#endif
    }

    explicit RP2(const R2Array &v): m_v{{v[0], v[1], 1.f}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2(const R2Array &)\n";
#endif
    }

    explicit RP2(const R2Tuple &v): m_v{{std::get<0>(v), std::get<1>(v), 1.f}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2(const R2Tuple &)\n";
#endif
    }

    // conversion from Euclidean point
    explicit RP2(const R2 &p, float w = 1.f): m_v{{p[0], p[1], w}} {
#ifdef POINT_DEBUG
        std::cerr << "RP2(R2 &, float = 1.f)\n";
#endif
    }

    // conversion to array and tuple
    explicit operator RP2Array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2::operator RP2Array()\n";
#endif
        return m_v;
    }

    explicit operator RP2Tuple() const {
#ifdef POINT_DEBUG
        std::cerr << "RP2::operator RP2Tuple()\n";
#endif
        return RP2Tuple{m_v[0], m_v[1], m_v[2]};
    }

    // conversion to array and tuple of other type
    explicit operator R2Array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2::operator R2Array()\n";
#endif
        return R2Array{{m_v[0]/m_v[2], m_v[1]/m_v[2]}};
    }

    // explicit conversion to array and tuple
    explicit operator R2Tuple() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2::operator R2Tuple()\n";
#endif
        return R2Tuple{m_v[0]/m_v[2], m_v[1]/m_v[2]};
    }

    // explicit conversion to Euclidean point
    explicit operator R2() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit RP2::operator R2()\n";
#endif
        return R2{m_v[0]/m_v[2], m_v[1]/m_v[2]};
    }

    // move the exponent and sign from w to x,y
    RP2 adjusted(void) const {
        int e;
        float f = std::abs(std::frexp(m_v[2], &e));
        e = -e;
        if (m_v[2] > 0) {
            return RP2{std::ldexp(m_v[0], e), std::ldexp(m_v[1], e), f};
        } else {
            return RP2{-std::ldexp(m_v[0], e), -std::ldexp(m_v[1], e), f};
        }
    }

    float x(void) const { return m_v[0]; }
    float y(void) const { return m_v[1]; }
    float z(void) const { return m_v[2]; }

private:
    friend IPoint<RP2>;

    float do_component(int i) const { return m_v[i]; }

    // note the difference between Euclidean and projective sum
    RP2 do_added(const RP2 &o) const {
        if (m_v[2] == o.m_v[2]) {
            return RP2{ m_v[0]+o.m_v[0], m_v[1]+o.m_v[1], m_v[2] };
        } else {
            return RP2{ m_v[0]*o.m_v[2]+o.m_v[0]*m_v[2],
               m_v[1]*o.m_v[2]+o.m_v[1]*m_v[2], m_v[2]*o.m_v[2]
            };
        }
    }

    // note the difference between Euclidean and projective subtraction
    RP2 do_subtracted(const RP2 &o) const {
        if (m_v[2] == o.m_v[2]) {
            return RP2{ m_v[0]-o.m_v[0], m_v[1]-o.m_v[1], m_v[2] };
        } else {
            return RP2{ m_v[0]*o.m_v[2]-o.m_v[0]*m_v[2],
               m_v[1]*o.m_v[2]-o.m_v[1]*m_v[2], m_v[2]*o.m_v[2]
            };
        }
    }

    // note the difference between Euclidean and projective
    // scalar multiplication
    RP2 do_scaled(float s) const {
        return RP2{ s*m_v[0], s*m_v[1], m_v[2] };
    }

    // note the difference between Euclidean and projective unary minus
    RP2 do_negated(void) const {
        return RP2{ -m_v[0], -m_v[1], m_v[2] };
    }

    bool do_is_almost_ideal(void) const {
        const float m = std::max(std::abs(m_v[0]), std::abs(m_v[1]));
        return std::abs(m_v[2]) < m*rvg::math::maxrel<float>();
    }

    bool do_is_equal(const RP2 &o) const {
        if (m_v[2] != 0.f || o.m_v[2] != 0.f) {
            return m_v[0]*o.m_v[2] == o.m_v[0]*m_v[2] &&
                   m_v[1]*o.m_v[2] == o.m_v[1]*m_v[2];
        } else {
            return m_v[0]*o.m_v[1] == o.m_v[0]*m_v[1];
        }
    }

    bool do_is_almost_equal(const RP2 &o) const {
        if (!is_almost_ideal() || !o.is_almost_ideal()) {
            return rvg::math::is_almost_equal(m_v[0]*o.m_v[2], o.m_v[0]*m_v[2])
                && rvg::math::is_almost_equal(m_v[1]*o.m_v[2], o.m_v[1]*m_v[2]);
        } else {
            return rvg::math::is_almost_equal(m_v[0]*o.m_v[1], o.m_v[0]*m_v[1]);
        }
    }

    RP2Tuple do_untie(void) const {
        return RP2Tuple{m_v[0], m_v[1], m_v[2]};
    }

    std::ostream &do_print(std::ostream &out) const {
        out << "rp2{" << m_v[0] << ',' << m_v[1] << ',' << m_v[2] << '}';
        return out;
    }
};


// Euclidean point in R3
class R3 final: public IPoint<R3> {

    R3Array m_v;

public:

    R3(void): m_v{{0.f, 0.f, 0.f}} {
#ifdef POINT_DEBUG
        std::cerr << "R3(void)\n";
#endif
    }

    R3(float x, float y, float z): m_v{{x, y, z}} {
#ifdef POINT_DEBUG
        std::cerr << "R3(float, float, float)\n";
#endif
    }

    // explicit conversion from RP2
    explicit R3(const RP2 &rp2): m_v{{rp2[0], rp2[1], rp2[2]}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3(const RP2 &)\n";
#endif
    }

    // conversion from array and tuple
    explicit R3(const R3Array &v): m_v(v) {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3(const R3Array &)\n";
#endif
    }

    explicit R3(const R3Tuple &v): m_v{{std::get<0>(v), std::get<1>(v), std::get<2>(v)}} {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3(const R3Tuple &)\n";
#endif
    }

    // automatic conversion to array and tuple
    explicit operator R3Array() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3::operator R3Array()\n";
#endif
        return m_v;
    }

    explicit operator R3Tuple() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3::operator R3Tuple()\n";
#endif
        return R3Tuple{m_v[0], m_v[1], m_v[2]};
    }

    // explicit conversion to RP2
    explicit operator RP2() const {
#ifdef POINT_DEBUG
        std::cerr << "explicit R3::operator RP2()\n";
#endif
        return RP2{m_v[0], m_v[1], m_v[2]};
    }

    float x(void) const { return m_v[0]; }
    float y(void) const { return m_v[1]; }
    float z(void) const { return m_v[2]; }

private:
    friend IPoint<R3>;

    R3 do_added(const R3 &o) const {
        return R3{ m_v[0]+o.m_v[0], m_v[1]+o.m_v[1], m_v[2]+o.m_v[2] };
    }

    R3 do_subtracted(const R3 &o) const {
        return R3{ m_v[0]-o.m_v[0], m_v[1]-o.m_v[1], m_v[2]-o.m_v[2]};
    }

    R3 do_negated() const {
        return R3{ -m_v[0], -m_v[1], -m_v[2] };
    }

    R3 do_scaled(float s) const {
        return R3{ s*m_v[0], s*m_v[1], s*m_v[2] };
    }

    float do_component(int i) const { return m_v[i]; }

    R3Tuple do_untie(void) const {
        return R3Tuple{m_v[0], m_v[1], m_v[2]};
    }

    bool do_is_equal(const R3 &o) const {
        return m_v[0] == o.m_v[0] && m_v[1] == o.m_v[1] && m_v[2] == o.m_v[2];
    }

    bool do_is_almost_equal(const R3 &o) const {
        return rvg::math::is_almost_equal(m_v[0], o.m_v[0]) &&
            rvg::math::is_almost_equal(m_v[1], o.m_v[1]) &&
            rvg::math::is_almost_equal(m_v[2], o.m_v[2]);
    }

    bool do_is_almost_ideal(void) const { return false; }

    std::ostream &print(std::ostream &out) const {
        out << "r3{" << m_v[0] << ',' << m_v[1] << ',' << m_v[2] << '}';
        return out;
    }
};

template <typename P, typename Q,
    typename = typename std::enable_if<
        rvg::meta::is_an_ipoint<P>::value &&
        rvg::meta::is_an_ipoint<Q>::value &&
        !std::is_same<P,Q>::value &&
        std::is_convertible<P,Q>::value>::type>
decltype(std::declval<Q&>().added(Q()))
operator+(const P &p, const Q &q) {
    return static_cast<Q>(p).added(q);
}

template <typename P, typename Q,
    typename = typename std::enable_if<
        rvg::meta::is_an_ipoint<P>::value &&
        rvg::meta::is_an_ipoint<Q>::value &&
        !std::is_same<P,Q>::value &&
        std::is_convertible<P,Q>::value>::type>
decltype(std::declval<Q&>().subtracted(Q()))
operator-(const P &p, const Q &q) {
    return static_cast<Q>(p).subtracted(q);
}

template <typename P,
    typename = typename std::enable_if<
        rvg::meta::is_an_ipoint<P>::value>::type>
P operator*(float s, const P &p) {
    return p.scaled(s);
}

template <typename P,
    typename = typename std::enable_if<
        rvg::meta::is_an_ipoint<P>::value>::type>
decltype(std::declval<P&>().untie())
untie(const P &p) {
    return p.untie();
}

using std::tie;

inline R2 perp(const R2 &p) {
    return R2{-p[1], p[0]};
}

inline RP2 perp(const RP2 &p) {
    return RP2{-p[1], p[0], p[2]};
}

inline float dot(const R2 &p, const R2 &q) {
    return p[0]*q[0] + p[1]*q[1];
}

inline float len2(const R2 &p) {
    return dot(p, p);
}

} } // namespace rvg::point

#endif
