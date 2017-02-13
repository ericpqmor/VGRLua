#ifndef RVG_XFORM_AFFINITY_H
#define RVG_XFORM_AFFINITY_H

#include "xform/ixform.h"

namespace rvg {
    namespace xform {

class Affinity final: public IXform<Affinity> {

    std::array<std::array<float,3>,2> m_m;

public:
    // constructors
    Affinity(const Affinity &a) = default;

    explicit Affinity(const std::array<std::array<float,3>,2> &m): m_m(m) {
#ifdef XFORM_DEBUG
        std::cerr << "Affinity(const std::array<std::array<float,3>,2> &)\n";
#endif
    }

    Affinity(const rvg::point::R2 &c0, const rvg::point::R2 &c1,
        const rvg::point::R2 &c2):
        Affinity{{{{c0[0],c1[0],c2[0]}, {c0[1],c1[1],c2[1]}}}} {
#ifdef XFORM_DEBUG
        std::cerr << "Affinity(const R2 &, const R2 &, const R2 &)\n";
#endif
    }

    Affinity(float a, float b, float tx, float c, float d, float ty):
        Affinity{{{{a,b,tx},{c,d,ty}}}} {
#ifdef XFORM_DEBUG
        std::cerr << "Affinity(float, ..., float)\n";
#endif
    }

    Affinity(): Affinity{{{{1.f,0.f,0.f},{0.f,1.f,0.f}}}} {
#ifdef XFORM_DEBUG
        std::cerr << "Affinity()\n";
#endif
    }

    // promotion
    operator Projectivity() const {
#ifdef XFORM_DEBUG
        std::cerr << "Affinity.operator Projectivity()\n";
#endif
        return Projectivity{{m_m[0], m_m[1], {0, 0, 1}}};
    }

    const std::array<float, 3> &operator[](int i) const;

private:
    friend IXform<Affinity>;

    rvg::point::RP2Tuple do_apply(float x, float y, float w) const;

    rvg::point::R2Tuple do_apply(float x, float y) const;

    rvg::point::RP2 do_apply(const rvg::point::RP2 &p) const;

    rvg::point::R2 do_apply(const rvg::point::R2 &e) const;

    Affinity do_transformed(const Affinity &o) const;

    Affinity do_rotated(float cos, float sin) const;

    Affinity do_scaled(float sx, float sy) const;

    Affinity do_translated(float tx, float ty) const;

    std::ostream &do_print(std::ostream &out) const;

    bool do_is_equal(const Affinity &o) const;

    bool do_is_almost_equal(const Affinity &o) const;

    bool do_is_identity(void) const;

    Projectivity do_adjugate(void) const;

    Projectivity do_transpose(void) const;

    Affinity do_inverse(void) const;

    float do_det(void) const;
};

} } // namespace rvg::xform

#endif
