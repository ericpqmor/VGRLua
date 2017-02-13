#ifndef RVG_XFORM_IXFORM_H
#define RVG_XFORM_IXFORM_H

#include <utility>
#include <iosfwd>

#include "meta/meta.h"
#include "point/point.h"
#include "math/math.h"

namespace rvg {
    namespace xform {

// ??D In c++14, we can infer the return type directly with
// auto, instead of using template methods...
// This is a better solution because it requires the derived
// classes to implement all methods that have their return types inferred
// #define HAS_CPP14_AUTO_RETURN
template <typename DERIVED>
class IXform {
    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }
public:
    // apply transformation to vector components
#ifdef HAS_CPP14_AUTO_RETURN
    auto apply(float x, float y, float w) const
#else
    template <typename D = DERIVED>
    auto apply(float x, float y, float w) const ->
        decltype(std::declval<const D&>().do_apply(float(), float(), float()))
#endif
    {
        return derived().do_apply(x, y, w);
    }

#ifdef HAS_CPP14_AUTO_RETURN
    auto apply(float x, float y) const
#else
    template <typename D = DERIVED>
    auto apply(float x, float y) const ->
        decltype(std::declval<const D&>().do_apply(float(), float()))
#endif
    {
        return derived().do_apply(x, y);
    }

    // apply transformation to vector
#ifdef HAS_CPP14_AUTO_RETURN
    auto apply(const rvg::point::RP2 &p) const
#else
    template <typename D = DERIVED>
    auto apply(const rvg::point::RP2 &p) const ->
        decltype(std::declval<const D&>().do_apply(rvg::point::RP2()))
#endif
    {
        return derived().do_apply(p);
    }

#ifdef HAS_CPP14_AUTO_RETURN
    auto apply(const rvg::point::R2 &e) const
#else
    template <typename D = DERIVED>
    auto apply(const rvg::point::R2 &e) const ->
        decltype(std::declval<const D&>().do_apply(rvg::point::R2()))
#endif
    {
        return derived().do_apply(e);
    }

    // apply rotation
#ifdef HAS_CPP14_AUTO_RETURN
    auto rotated(float deg) const
#else
    template <typename D = DERIVED>
    auto rotated(float deg) const ->
        decltype(std::declval<const D&>().do_rotated(float(), float()))
#endif
    {
        float rad = rvg::math::rad(deg);
        return derived().do_rotated(std::cos(rad), std::sin(rad));
    }

    // apply rotation
#ifdef HAS_CPP14_AUTO_RETURN
    auto rotated(float cos, float sin) const
#else
    template <typename D = DERIVED>
    auto rotated(float cos, float sin) const ->
        decltype(std::declval<const D&>().do_rotated(float(), float()))
#endif
    {
        return derived().do_rotated(cos, sin);
    }

    // apply scaling
#ifdef HAS_CPP14_AUTO_RETURN
    auto scaled(float sx, float sy) const
#else
    template <typename D = DERIVED>
    auto scaled(float sx, float sy) const ->
        decltype(std::declval<const D&>().do_scaled(float(), float()))
#endif
    {
        return derived().do_scaled(sx, sy);
    }

#ifdef HAS_CPP14_AUTO_RETURN
    auto scaled(float s) const
#else
    template <typename D = DERIVED>
    auto scaled(float s) const ->
        decltype(std::declval<const D&>().do_scaled(float(), float()))
#endif
    {
        return derived().do_scaled(s, s);
    }

    // apply translation
#ifdef HAS_CPP14_AUTO_RETURN
    auto translated(float tx, float ty) const
#else
    template <typename D = DERIVED>
    auto translated(float tx, float ty) const ->
        decltype(std::declval<const D&>().do_translated(float(), float()))
#endif
    {
        return derived().do_translated(tx, ty);
    }

    // apply derived transformation to derived transformation
    DERIVED transformed(const DERIVED &o) const {
        return derived().do_transformed(o);
    }

    DERIVED operator*(const DERIVED &o) const {
        return o.transformed(derived());
    }

#ifdef HAS_CPP14_AUTO_RETURN
    auto operator*(float s) const
#else
    template <typename D = DERIVED>
    auto operator*(float s) const ->
        decltype(std::declval<const D&>().do_scaled(float()))
#endif
    {
        return derived().do_scaled(s);
    }

#ifdef HAS_CPP14_AUTO_RETURN
    auto operator*(const rvg::point::R2 &r2) const
#else
    template <typename D = DERIVED>
    auto operator*(const rvg::point::R2 &r2) const ->
        decltype(std::declval<const D&>().do_apply(rvg::point::R2()))
#endif
    {
        return derived().do_apply(r2);
    }

#ifdef HAS_CPP14_AUTO_RETURN
    auto operator*(const rvg::point::RP2 &rp2) const
#else
    template <typename D = DERIVED>
    auto operator*(const rvg::point::RP2 &rp2) const ->
        decltype(std::declval<const D&>().do_apply(rvg::point::RP2()))
#endif
    {
        return derived().do_apply(rp2);
    }

    bool operator==(const DERIVED &o) const {
        return derived().do_is_equal(o);
    }

    bool operator!=(const DERIVED &o) const {
        return !derived().do_is_equal(o);
    }

    // print to ostream
    std::ostream &print(std::ostream &out) const {
        return derived().do_print(out);
    }

    bool is_almost_equal(const DERIVED &o) const {
        return derived().do_is_almost_equal(o);
    }

    bool is_equal(const DERIVED &o) const {
        return derived().do_is_equal(o);
    }

    float det(void) const {
        return derived().do_det();
    }

    // return matrix adjugate where A * A.ajugate() == A.det() * Identity()
#ifdef HAS_CPP14_AUTO_RETURN
    auto adjugate(void) const
#else
    template <typename D = DERIVED>
    auto adjugate(void) const ->
        decltype(std::declval<const D&>().do_adjugate())
#endif
    {
        return derived().do_adjugate();
    }

    // return matrix transform
#ifdef HAS_CPP14_AUTO_RETURN
    auto transpose(void) const
#else
    template <typename D = DERIVED>
    auto transpose(void) const ->
        decltype(std::declval<const D&>().do_transpose())
#endif
    {
        return derived().do_transpose();
    }

    // compute inverse transformation
    DERIVED inverse(void) const {
        return derived().do_inverse();
    }

    bool is_identity(void) const {
        return derived().do_is_identity();
    }
};

template <typename DERIVED>
std::ostream &operator<<(std::ostream &out, const IXform<DERIVED> &xf) {
    return xf.print(out);
}

} } // namespace rvg::xform

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_ixform = std::integral_constant<
    bool,
    is_crtp_of<
        rvg::xform::IXform,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // rvg::meta

#endif
