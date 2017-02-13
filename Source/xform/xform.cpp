#include <iostream>

#include "xform/xform.h"

namespace rvg {
    namespace xform {

std::ostream &Projectivity::do_print(std::ostream &out) const {
    out << "projectivity{" <<
        m_m[0][0] << ',' << m_m[0][1] << ',' << m_m[0][2] << ',' <<
        m_m[1][0] << ',' << m_m[1][1] << ',' << m_m[1][2] << ',' <<
        m_m[2][0] << ',' << m_m[2][1] << ',' << m_m[2][2] << '}';
    return out;
}

std::ostream &Affinity::do_print(std::ostream &out) const {
    out << "affinity{" <<
        m_m[0][0] << ',' << m_m[0][1] << ',' << m_m[0][2] << ',' <<
        m_m[1][0] << ',' << m_m[1][1] << ',' << m_m[1][2] << '}';
    return out;
}

std::ostream &Linear::do_print(std::ostream &out) const {
    out << "linear{" <<
        m_m[0][0] << ',' << m_m[0][1] << ',' <<
        m_m[1][0] << ',' << m_m[1][1] << '}';
    return out;
}

std::ostream &Rotation::do_print(std::ostream &out) const {
    out << "rotation{" << m_cos << ',' << m_sin << '}';
    return out;
}

std::ostream &Translation::do_print(std::ostream &out) const {
    out << "translation{" << m_tx << ',' << m_ty << '}';
    return out;
}

std::ostream &Scaling::do_print(std::ostream &out) const {
    out << "scaling{" << m_sx << ',' << m_sy << '}';
    return out;
}

std::ostream &Identity::do_print(std::ostream &out) const {
    out << "identity{}";
    return out;
}

Affinity windowviewport(const rvg::bbox::Window &window,
const rvg::bbox::Viewport &viewport, Align align_x, Align align_y,
Aspect aspect) {
    float wxl, wyb, wxr, wyt;
    std::tie(wxl, wyb) = window.bl();
    std::tie(wxr, wyt) = window.tr();
    float wdx = wxr - wxl;
    float wdy = wyt - wyb;
    int vxl, vyb, vxr, vyt;
    std::tie(vxl, vyb) = viewport.bl();
    std::tie(vxr, vyt) = viewport.tr();
    float vdx = static_cast<float>(vxr - vxl);
    float vdy = static_cast<float>(vyt - vyb);
    float xi = 0.f, xo = 0.f;
    switch (align_x) {
        case Align::min:
            xi = wxl;
            xo = static_cast<float>(vxl);
            break;
        case Align::mid:
            xi = .5f*(wxl+wxr);
            xo = .5f*(static_cast<float>(vxl+vxr));
            break;
        case Align::max:
            xi = wxr;
            xo = static_cast<float>(vxr);
            break;
    }
    float yi = 0.f, yo = 0.f;
    switch (align_y) {
        case Align::min:
            yi = wyb;
            yo = static_cast<float>(vyb);
            break;
        case Align::mid:
            yi = .5f*(wyb+wyt);
            yo = .5f*(static_cast<float>(vyb+vyt));
            break;
        case Align::max:
            yi = wyt;
            yo = static_cast<float>(vyt);
            break;
    }
    float sx = 0.f, sy = 0.f;
    switch (aspect) {
        case Aspect::none:
            sx = static_cast<float>(vdx)/wdx;
            sy = static_cast<float>(vdy)/wdy;
            break;
        case Aspect::extend:
            // if (wdy/wdx < vdy/vdx) {
            if (std::abs(wdy*vdx) < std::abs(vdy*wdx)) {
                sy = static_cast<float>(vdy)/wdy;
                sx = sy*wdx/wdy;
            } else {
                sx = static_cast<float>(vdx)/wdx;
                sy = sx*wdy/wdx;
            }
            break;
        case Aspect::trim:
            if (std::abs(wdy*vdx) < std::abs(vdy*wdx)) {
                sx = static_cast<float>(vdx)/wdx;
                sy = sx*wdy/wdx;
            } else {
                sy = static_cast<float>(vdy)/wdy;
                sx = sy*wdx/wdy;
            }
            break;
    }

    return Translation(-xi, -yi).scaled(sx, sy).translated(xo, yo);
}

} } // namespace rvg::xform
