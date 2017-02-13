#ifndef RVG_XFORM_TRANSLATION_H
#define RVG_XFORM_TRANSLATION_H

#include "xform/ixform.h"

namespace rvg {
    namespace xform {

class Translation final: public IXform<Translation> {

    float m_tx, m_ty;

public:
    // constructors
    Translation(const Translation &t) = default;

    Translation(float tx, float ty): m_tx(tx), m_ty(ty) {
#ifdef XFORM_DEBUG
        std::cerr << "Translation(float, float)\n";
#endif
    }

    Translation(): Translation(0.f, 0.f) {
#ifdef XFORM_DEBUG
        std::cerr << "Translation()\n";
#endif
    }

    // promotions
    operator Affinity() const {
#ifdef XFORM_DEBUG
        std::cerr << "Translation.operator Affinity()\n";
#endif
        return Affinity(1.f, 0.f, m_tx, 0.f, 1.f, m_ty);
    }

    operator Projectivity() const {
#ifdef XFORM_DEBUG
        std::cerr << "Translation.operator Projectivity()\n";
#endif
        return Projectivity(1.f, 0.f, m_tx, 0.f, 1.f, m_ty, 0.f, 0.f, 1.f);
    }

    float tx(void) const;

    float ty(void) const;

private:
    friend IXform<Translation>;

    rvg::point::RP2Tuple do_apply(float x, float y, float w) const;

    rvg::point::R2Tuple do_apply(float x, float y) const;

    rvg::point::RP2 do_apply(const rvg::point::RP2 &p) const;

    rvg::point::R2 do_apply(const rvg::point::R2 &e) const;

    Translation do_transformed(const Translation &o) const;

    Affinity do_rotated(float cos, float sin) const;

    Affinity do_scaled(float sx, float sy) const;

    Translation do_translated(float tx, float ty) const;

    std::ostream &do_print(std::ostream &out) const;

    bool do_is_equal(const Translation &o) const;

    bool do_is_almost_equal(const Translation &o) const;

    bool do_is_identity(void) const;

    Projectivity do_transpose(void) const;

    Translation do_adjugate(void) const;

    Translation do_inverse(void) const;

    float do_det(void) const;
};

} } // namespace rvg::xform

#endif
