#ifndef RVG_PATH_FILTER_CLOSE_PATH_H
#define RVG_PATH_FILTER_CLOSE_PATH_H

#include <string>
#include <iosfwd>
#include <type_traits>
#include <utility>

#include "path/filter/forwarder.h"

namespace rvg {
    namespace path {
        namespace filter {

// Make sure closed contours contain the segment connecting
// the last point to the first point
// Alternative, transform open contours into closed
// contours.
template <typename SINK, bool ALL>
class ClosePath final: public Forwarder<ClosePath<SINK,ALL>, SINK> {
public:

    using Base = Forwarder<ClosePath<SINK,ALL>, SINK>;

    explicit ClosePath(SINK &&sink): Base(std::forward<SINK>(sink)) {
        static_assert(rvg::meta::is_an_ipath<SINK>::value,
            "sink is not an IPath");
    }

private:
    friend IPath<ClosePath<SINK,ALL>>;

    void do_begin_closed_contour(int len, float x0, float y0) {
        m_first_x = x0; m_first_y = y0;
        return Base::do_begin_closed_contour(len, x0, y0);
    }

    void do_begin_open_contour(int len, float x0, float y0) {
        if (ALL) return do_begin_closed_contour(len, x0, y0);
        else return Base::do_begin_open_contour(len, x0, y0);
    }

    void do_end_closed_contour(float x0, float y0, int len) {
        if (m_first_x != x0 || m_first_y != y0) {
            Base::do_linear_segment(x0, y0, m_first_x, m_first_y);
            return Base::do_end_closed_contour(m_first_x, m_first_y, len+1);
        } else {
            return Base::do_end_closed_contour(x0, y0, len);
        }
    }

    void do_end_open_contour(float x0, float y0, int len) {
        if (ALL) return do_end_closed_contour(x0, y0, len);
        else return Base::do_end_open_contour(x0, y0, len);
    }

private:
    float m_first_x, m_first_y;
};

template <typename SINK>
ClosePath<SINK,false> make_close_path(SINK &&sink) {
    return ClosePath<SINK,false>(std::forward<SINK>(sink));
}

template <typename SINK>
ClosePath<SINK,true> make_close_all_path(SINK &&sink) {
    return ClosePath<SINK,true>(std::forward<SINK>(sink));
}

} } } // namespace rvg::path::filter

#endif
