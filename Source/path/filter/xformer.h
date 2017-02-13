#ifndef RVG_PATH_FILTER_XFORMER_H
#define RVG_PATH_FILTER_XFORMER_H

#include <type_traits>
#include <utility>

#include "path/filter/forwarder.h"
#include "xform/xform.h"

namespace rvg {
    namespace path {
        namespace filter {

template <typename FORWARD>
class Xformer final: public Forwarder<Xformer<FORWARD>, FORWARD> {

    using Xform = rvg::xform::Xform;

    const Xform &m_xf;

    float m_x, m_y;

public:
    using Base = Forwarder<Xformer<FORWARD>, FORWARD>;

    explicit Xformer(const Xform &xf, FORWARD &&f):
    Base(std::forward<FORWARD>(f)), m_xf(xf) { ; }

private:

    friend IPath<Xformer<FORWARD>>;

    void do_linear_segment(float x0, float y0, float x1, float y1) {
        x0 = m_x; y0 = m_y;
        std::tie(x1, y1) = m_xf.apply(x1, y1);
        m_x = x1; m_y = y1;
        return Base::do_linear_segment(x0, y0, x1, y1);
    }

    void do_linear_segment_with_length(float x0, float y0, float len,
        float x1, float y1) {
        (void) len;
        x0 = m_x; y0 = m_y;
        std::tie(x1, y1) = m_xf.apply(x1, y1);
        m_x = x1; m_y = y1;
        return Base::do_linear_segment_with_length(x0, y0,
            rvg::math::len(x1-x0, y1-y0), x1, y1);
    }

    void do_degenerate_segment(float x0, float y0, float dx0, float dy0,
            float dx1, float dy1, float x1, float y1) {
        x0 = m_x; y0 = m_y;
        std::tie(dx0, dy0, std::ignore) = m_xf.apply(dx0, dy0, 0);
        std::tie(dx1, dy1, std::ignore) = m_xf.apply(dx1, dy1, 0);
        std::tie(x1, y1) = m_xf.apply(x1, y1);
        m_x = x1; m_y = y1;
        return Base::do_degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1);
    }

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
        x0 = m_x; y0 = m_y;
        std::tie(x1, y1) = m_xf.apply(x1, y1);
        std::tie(x2, y2) = m_xf.apply(x2, y2);
        m_x = x2; m_y = y2;
        return Base::do_quadratic_segment(x0, y0, x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment(float x0, float y0, float x1, float y1,
        float w1, float x2, float y2) {
        x0 = m_x; y0 = m_y;
        std::tie(x1, y1, w1) = m_xf.apply(x1, y1, w1);
        std::tie(x2, y2) = m_xf.apply(x2, y2);
        m_x = x2; m_y = y2;
        return Base::do_rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2);
    }

    void do_cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
        x0 = m_x; y0 = m_y;
        std::tie(x1, y1) = m_xf.apply(x1, y1);
        std::tie(x2, y2) = m_xf.apply(x2, y2);
        std::tie(x3, y3) = m_xf.apply(x3, y3);
        m_x = x3; m_y = y3;
        return Base::do_cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3);
    }

    void do_begin_segment(float ddx, float ddy, float ddw, float dx,
        float dy, float dw, float x, float y) {
        std::tie(x, y) = m_xf.apply(x, y);
        std::tie(dx, dy, dw) = m_xf.apply(dx, dy, dw);
        std::tie(ddx, ddy, ddw) = m_xf.apply(ddx, ddy, ddw);
        return Base::do_begin_segment(ddx, ddy, ddw, dx, dy, dw, x, y);
    }

    void do_end_segment(float ddx, float ddy, float ddw, float dx,
        float dy, float dw, float x, float y) {
        std::tie(x, y) = m_xf.apply(x, y);
        std::tie(dx, dy, dw) = m_xf.apply(dx, dy, dw);
        std::tie(ddx, ddy, ddw) = m_xf.apply(ddx, ddy, ddw);
        return Base::do_end_segment(ddx, ddy, ddw, dx, dy, dw, x, y);
    }

    void do_begin_open_contour(int len, float x0, float y0) {
        std::tie(x0, y0) = m_xf.apply(x0, y0);
        m_x = x0; m_y = y0;
        return Base::do_begin_open_contour(len, x0, y0);
    }

    void do_end_open_contour(float x0, float y0, int len) {
        x0 = m_x; y0 = m_y;
        return Base::do_end_open_contour(x0, y0, len);
    }

    void do_begin_closed_contour(int len, float x0, float y0) {
        std::tie(x0, y0) = m_xf.apply(x0, y0);
        m_x = x0; m_y = y0;
        return Base::do_begin_closed_contour(len, x0, y0);
    }

    void do_end_closed_contour(float x0, float y0, int len) {
        x0 = m_x; y0 = m_y;
        return Base::do_end_closed_contour(x0, y0, len);
    }

    void do_begin_dash(int len, float x0, float y0) {
        std::tie(x0, y0) = m_xf.apply(x0, y0);
        return Base::do_begin_dash(len, x0, y0);
    }

    void do_end_dash(float x0, float y0, int len) {
        std::tie(x0, y0) = m_xf.apply(x0, y0);
        return Base::do_end_dash(x0, y0, len);
    }
};

template <typename FORWARD>
Xformer<FORWARD> make_xformer(const rvg::xform::Xform &xf, FORWARD &&f) {
    return Xformer<FORWARD>(xf, std::forward<FORWARD>(f));
}

} } } // namespace rvg::path::filter

#endif
