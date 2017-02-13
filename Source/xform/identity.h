#ifndef RVG_XFORM_IDENTITY_H
#define RVG_XFORM_IDENTITY_H

#include "xform/ixform.h"

namespace rvg {
    namespace xform {

class Identity final: public IXform<Identity> {
public:
    // promotions
    operator Scaling() const {
#ifdef XFORM_DEBUG
        std::cerr << "Identity.operator Scaling()\n";
#endif
        return Scaling(1.0f, 1.0f);
    }

    operator Rotation() const {
#ifdef XFORM_DEBUG
        std::cerr << "Identity.operator Rotation()\n";
#endif
        return Rotation(1.f, 0.f);
    }

    operator Translation() const {
#ifdef XFORM_DEBUG
        std::cerr << "Identity.operator Translation()\n";
#endif
        return Translation(0.f, 0.f);
    }

    operator Linear() const {
#ifdef XFORM_DEBUG
        std::cerr << "Identity.operator Linear()\n";
#endif
        return Linear(1.f, 0.f, 0.f, 1.f);
    }

    operator Affinity() const {
#ifdef XFORM_DEBUG
        std::cerr << "Identity.operator Affinity()\n";
#endif
        return Affinity(1.f, 0.f, 0.f, 0.f, 1.f, 0.f);
    }

    operator Projectivity() const {
#ifdef XFORM_DEBUG
        std::cerr << "Identity.operator Projectivity()\n";
#endif
        return Projectivity(1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f);
    }

    // constructors
    Identity() = default;

private:
    friend IXform<Identity>;

    rvg::point::RP2Tuple do_apply(float x, float y, float w) const;

    rvg::point::R2Tuple do_apply(float x, float y) const;

    rvg::point::RP2 do_apply(const rvg::point::RP2 &p) const;

    rvg::point::R2 do_apply(const rvg::point::R2 &e) const;

    Identity do_transformed(const Identity &) const;

    Rotation do_rotated(float cos, float sin) const;

    Translation do_translated(float tx, float ty) const;

    Scaling do_scaled(float sx, float sy) const;

    std::ostream &do_print(std::ostream &out) const;

    bool do_is_equal(const Identity &) const;

    bool do_is_almost_equal(const Identity &) const;

    bool do_is_identity(void) const;

    Identity do_transpose(void) const;

    Identity do_adjugate(void) const;

    Identity do_inverse(void) const;

    float do_det(void) const;
};

} } // namespace rvg::xform

#endif
