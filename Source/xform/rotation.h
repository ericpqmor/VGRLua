#ifndef RVG_XFORM_ROTATION_H
#define RVG_XFORM_ROTATION_H

#include "math/math.h"

#include "xform/ixform.h"

namespace rvg {
    namespace xform {

class Rotation final: public IXform<Rotation> {

    float m_cos, m_sin;

public:
    // constructors
    Rotation(const Rotation &r) = default;

    Rotation(float cos, float sin): m_cos(cos), m_sin(sin) {
#ifdef XFORM_DEBUG
        std::cerr << "Rotation(float, float)\n";
#endif
    }

    Rotation(): Rotation{1.f, 0.f} {
#ifdef XFORM_DEBUG
        std::cerr << "Rotation()\n";
#endif
    }

    explicit Rotation(float deg) {
        float rad = rvg::math::rad(deg);
        m_cos = std::cos(rad);
        m_sin = std::sin(rad);
#ifdef XFORM_DEBUG
        std::cerr << "explicit Rotation(float)\n";
#endif
    }

    // promotions
    operator Linear() const {
#ifdef XFORM_DEBUG
        std::cerr << "Rotation.operator Linear()\n";
#endif
        return Linear(m_cos, -m_sin, m_sin, m_cos);
    }

    operator Affinity() const {
#ifdef XFORM_DEBUG
        std::cerr << "Rotation.operator Affinity()\n";
#endif
        return Affinity(m_cos, -m_sin, 0.f, m_sin, m_cos, 0.f);
    }

    operator Projectivity() const {
#ifdef XFORM_DEBUG
        std::cerr << "Rotation.operator Projectivity()\n";
#endif
        return Projectivity(m_cos, -m_sin, 0.f, m_sin, m_cos, 0.f, 0.f, 0.f, 1.f);
    }

    float cos(void) const;

    float sin(void) const;

private:
    friend IXform<Rotation>;

    rvg::point::RP2Tuple do_apply(float x, float y, float w) const;

    rvg::point::R2Tuple do_apply(float x, float y) const;

    rvg::point::RP2 do_apply(const rvg::point::RP2 &p) const;

    rvg::point::R2 do_apply(const rvg::point::R2 &e) const;

    Rotation do_transformed(const Rotation &o) const;

    Rotation do_rotated(float cos, float sin) const;

    Linear do_scaled(float sx, float sy) const;

    Affinity do_translated(float tx, float ty) const;

    std::ostream &do_print(std::ostream &out) const;

    bool do_is_equal(const Rotation &o) const;

    bool do_is_almost_equal(const Rotation &o) const;

    Rotation do_transpose(void) const;

    bool do_is_identity(void) const;

    Projectivity do_adjugate(void) const;

    Rotation do_inverse(void) const;

    float do_det(void) const;
};

} } // namespace rvg::xform

#endif
