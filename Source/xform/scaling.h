#ifndef RVG_XFORM_SCALING_H
#define RVG_XFORM_SCALING_H

#include "xform/ixform.h"

namespace rvg {
    namespace xform {

class Scaling final: public IXform<Scaling> {

    float m_sx, m_sy;

public:
    // constructors
    Scaling(const Scaling &s) = default;

    Scaling(float sx, float sy): m_sx(sx), m_sy(sy) {
#ifdef XFORM_DEBUG
        std::cerr << "Scaling(float, float)\n";
#endif
    }

    explicit Scaling(float s): Scaling(s, s) {
#ifdef XFORM_DEBUG
        std::cerr << "explicit Scaling(float)\n";
#endif
    }

    Scaling(): Scaling(1.f, 1.f) {
#ifdef XFORM_DEBUG
        std::cerr << "Scaling()\n";
#endif
    }

    // promotions
    operator Linear() const {
#ifdef XFORM_DEBUG
        std::cerr << "Scaling.operator Linear()\n";
#endif
        return Linear(m_sx, 0.f, 0.f, m_sy);
    }

    operator Affinity() const {
#ifdef XFORM_DEBUG
        std::cerr << "Scaling.operator Affinity()\n";
#endif
        return Affinity(m_sx, 0.f, 0.f, 0.f, m_sy, 0.f);
    }

    operator Projectivity() const {
#ifdef XFORM_DEBUG
        std::cerr << "Scaling.operator Projectivity()\n";
#endif
        return Projectivity(m_sx, 0.f, 0.f, 0.f, m_sy, 0.f, 0.f, 0.f, 1.f);
    }

    float sx(void) const;

    float sy(void) const;

private:
    friend IXform<Scaling>;

    rvg::point::RP2Tuple do_apply(float x, float y, float w) const;

    rvg::point::R2Tuple do_apply(float x, float y) const;

    rvg::point::RP2 do_apply(const rvg::point::RP2 &p) const;

    rvg::point::R2 do_apply(const rvg::point::R2 &e) const;

    Scaling do_transformed(const Scaling &o) const;

    Linear do_rotated(float cos, float sin) const;

    Scaling do_scaled(float sx, float sy) const;

    Affinity do_translated(float tx, float ty) const;

    std::ostream &do_print(std::ostream &out) const;

    bool do_is_equal(const Scaling &o) const;

    bool do_is_almost_equal(const Scaling &o) const;

    Projectivity do_adjugate(void) const;

    bool do_is_identity(void) const;

    Scaling do_transpose(void) const;

    Scaling do_inverse(void) const;

    float do_det(void) const;

};

} } // namespace rvg::xform

#endif
