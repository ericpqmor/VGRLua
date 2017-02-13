#ifndef RVG_XFORM_LINEAR_H
#define RVG_XFORM_LINEAR_H

#include <array>

#include "xform/ixform.h"

namespace rvg {
    namespace xform {

class Linear final: public IXform<Linear> {

    std::array<std::array<float,2>,2> m_m;

public:
    // constructors
    Linear(const Linear &l) = default;

    explicit Linear(const std::array<std::array<float,2>,2> &m): m_m(m) {
#ifdef XFORM_DEBUG
        std::cerr << "Linear(const std::array<std::array<float,2>,2> &)\n";
#endif
    }

    Linear(const rvg::point::R2 &c0, const rvg::point::R2 &c1):
        Linear{{{{c0[0],c1[0]}, {c0[1],c1[1]}}}} {
#ifdef XFORM_DEBUG
        std::cerr << "Linear(const R2 &, const R2 &)\n";
#endif
    }

    Linear(float a, float b, float c, float d): Linear{{{{a, b},{c, d}}}} {
#ifdef XFORM_DEBUG
        std::cerr << "Linear(float, ..., float)\n";
#endif
    }

    Linear(): Linear{{{{1.f, 0.f}, {0.f, 1.f}}}} {
#ifdef XFORM_DEBUG
        std::cerr << "Linear()\n";
#endif
    }

    // promotions
    operator Affinity() const {
#ifdef XFORM_DEBUG
        std::cerr << "Linear.operator Affinity()\n";
#endif
        return Affinity{m_m[0][0], m_m[0][1], 0.f, m_m[1][0], m_m[1][1], 0.f};
    }

    operator Projectivity() const {
#ifdef XFORM_DEBUG
        std::cerr << "Linear.operator Projectivity()\n";
#endif
        return Projectivity{m_m[0][0], m_m[0][1], 0.f, m_m[1][0], m_m[1][1], 0.f, 0.f, 0.f, 1.f};
    }

    const std::array<float,2> &operator[](int i) const;

private:
    friend IXform<Linear>;

    rvg::point::RP2Tuple do_apply(float x, float y, float w) const;

    rvg::point::R2Tuple do_apply(float x, float y) const;

    rvg::point::RP2 do_apply(const rvg::point::RP2 &p) const;

    rvg::point::R2 do_apply(const rvg::point::R2 &e) const;

    Linear do_transformed(const Linear &o) const;

    Linear do_rotated(float cos, float sin) const;

    Linear do_scaled(float sx, float sy) const;

    Affinity do_translated(float tx, float ty) const;

    std::ostream &do_print(std::ostream &out) const;

    bool do_is_equal(const Linear &o) const;

    bool do_is_almost_equal(const Linear &o) const;

    bool do_is_identity(void) const;

    Projectivity do_adjugate(void) const;

    Linear do_transpose(void) const;

    Linear do_inverse(void) const;

    float do_det(void) const;
};

} } // namespace rvg::xform

#endif
