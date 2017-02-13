#ifndef RVG_XFORM_PROJECTIVITY_H
#define RVG_XFORM_PROJECTIVITY_H

#include "xform/ixform.h"

namespace rvg {
    namespace xform {

class Projectivity final: public IXform<Projectivity> {

    std::array<std::array<float,3>,3> m_m;

public:
    // constructors
    Projectivity(const Projectivity &p) = default;

    explicit Projectivity(const std::array<std::array<float,3>,3> &m): m_m(m) {
#ifdef XFORM_DEBUG
        std::cerr << "Projectivity(const std::array<std::array<float,3>,3> &)\n";
#endif
    }

    Projectivity(const rvg::point::R3 &c0, const rvg::point::R3 &c1,
        const rvg::point::R3 &c2):
        Projectivity{{{{c0[0],c1[0],c2[0]}, {c0[1],c1[1],c2[1]},
           {c0[2],c1[2],c2[2]}}}} {
#ifdef XFORM_DEBUG
        std::cerr << "Affinity(const R3 &, const R3 &, const R3 &)\n";
#endif
    }

    Projectivity(float a, float b, float c, float d, float e, float f,
        float g, float h, float i):
        Projectivity{{{{a, b, c}, {d, e, f}, {g, h, i}}}} {
#ifdef XFORM_DEBUG
        std::cerr << "Projectivity(float, ..., float)\n";
#endif
    }

    Projectivity():
        Projectivity{{{{1.f,0.f,0.f},{0.f,1.f,0.f},{0.f,0.f,1.f}}}} {
#ifdef XFORM_DEBUG
        std::cerr << "Projectivity()\n";
#endif
    }

    const std::array<float, 3> &operator[](int i) const;

private:
    friend IXform<Projectivity>;

    rvg::point::RP2Tuple do_apply(float x, float y, float w) const;

    rvg::point::RP2Tuple do_apply(float x, float y) const;

    rvg::point::RP2 do_apply(const rvg::point::RP2 &p) const;

    rvg::point::RP2 do_apply(const rvg::point::R2 &e) const;

    Projectivity do_transformed(const Projectivity &o) const;

    Projectivity do_rotated(float cos, float sin) const;

    Projectivity do_scaled(float sx, float sy) const;

    Projectivity do_translated(float tx, float ty) const;

    std::ostream &do_print(std::ostream &out) const;

    bool do_is_equal(const Projectivity &o) const;

    bool do_is_almost_equal(const Projectivity &o) const;

    bool do_is_identity(void) const;

    Projectivity do_transpose(void) const;

    Projectivity do_adjugate(void) const;

    Projectivity do_inverse(void) const;

    float do_det(void) const;
};


} } // namespace rvg::xform

#endif
