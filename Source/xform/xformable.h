#ifndef RVG_XFORM_XFORMABLE_H
#define RVG_XFORM_XFORMABLE_H

#include "xform/xform.h"

namespace rvg {
    namespace xform {

template <typename DERIVED>
class Xformable {
public:
    Xformable(void): m_xf(Identity{}) { ; }
    Xformable(const Xformable &) = default;
    Xformable(Xformable &&) = default;
    Xformable<DERIVED> &operator=(const Xformable<DERIVED> &) = default;
    Xformable<DERIVED> &operator=(Xformable<DERIVED> &&) = default;

    DERIVED rotated(float deg) const {
        DERIVED copy = *static_cast<const DERIVED *>(this);
        copy.set_xf(m_xf.rotated(deg));
        return copy;
    }

    DERIVED rotated(float deg, float cx, float cy) const {
        DERIVED copy = *static_cast<const DERIVED *>(this);
        copy.set_xf(m_xf.translated(-cx, -cy).rotated(deg).translated(cx, cy));
        return copy;
    }

    DERIVED translated(float tx, float ty) const {
        DERIVED copy = *static_cast<const DERIVED *>(this);
        copy.set_xf(m_xf.translated(tx, ty));
        return copy;
    }

    DERIVED scaled(float sx, float sy, float cx, float cy) const {
        DERIVED copy = *static_cast<const DERIVED *>(this);
        copy.set_xf(m_xf.translated(-cx, -cy).scaled(sx, sy).
            translated(cx, cy));
        return copy;
    }

    DERIVED scaled(float s, float cx, float cy) const {
        DERIVED copy = *static_cast<const DERIVED *>(this);
        copy.set_xf(m_xf.translated(-cx, -cy).scaled(s).
            translated(cx, cy));
        return copy;
    }

    DERIVED scaled(float sx, float sy) const {
        DERIVED copy = *static_cast<const DERIVED *>(this);
        copy.set_xf(m_xf.scaled(sx, sy));
        return copy;
    }

    DERIVED scaled(float s) const {
        DERIVED copy = *static_cast<const DERIVED *>(this);
        copy.set_xf(m_xf.scaled(s, s));
        return copy;
    }

    DERIVED transformed(const Xform &other) const {
        DERIVED copy = *static_cast<const DERIVED *>(this);
        copy.set_xf(m_xf.transformed(other));
        return copy;
    }

    DERIVED affine(float a, float b, float tx, float c, float d, float ty)
    const {
        DERIVED copy = *static_cast<const DERIVED *>(this);
        copy.set_xf(m_xf.transformed(Affinity(a, b, tx, c, d, ty)));
        return copy;
    }

    DERIVED linear(float a, float b, float c, float d) const {
        DERIVED copy = *static_cast<const DERIVED *>(this);
        copy.set_xf(m_xf.transformed(Linear(a, b, c, d)));
        return copy;
    }

    DERIVED windowviewport(const rvg::bbox::Window &window,
        const rvg::bbox::Viewport &viewport, Align align_x = Align::mid,
        Align align_y = Align::mid, Aspect aspect = Aspect::none) const {
        DERIVED copy = *static_cast<const DERIVED *>(this);
        Xform wv = rvg::xform::windowviewport(window, viewport, align_x,
            align_y, aspect);
        copy.set_xf(wv * m_xf);
        return copy;
    }

    const Xform &xf(void) const {
        return m_xf;
    }

    void set_xf(const Xform &xf) {
        m_xf = xf;
    }

private:
    Xform m_xf;
};

} } // namespace rvg::xform

#endif
