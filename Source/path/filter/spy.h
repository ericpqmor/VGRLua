#ifndef RVG_PATH_FILTER_SPY_H
#define RVG_PATH_FILTER_SPY_H

#include <string>
#include <iosfwd>
#include <type_traits>
#include <utility>

#include "path/filter/forwarder.h"
#include "path/filter/null.h"

namespace rvg {
    namespace path {
        namespace filter {

template <typename SINK>
class Spy final: public Forwarder<Spy<SINK>, SINK> {
public:

    using Base = Forwarder<Spy<SINK>, SINK>;

    explicit Spy(const char *name, SINK &&sink, std::ostream &out):
    Base(std::forward<SINK>(sink)), m_name(name), m_out(out) {
        static_assert(rvg::meta::is_an_ipath<SINK>::value,
            "sink is not an IPath");
    }

private:
    friend IPath<Spy<SINK>>;

    void do_linear_segment(float x0, float y0, float x1, float y1) {
        m_out << m_name << "linear_segment("
            << x0 << ", " << y0 << ", "
            << x1 << ", " << y1 << ")\n";
        return Base::do_linear_segment(x0, y0, x1, y1);
    }

    void do_linear_segment_with_length(float x0, float y0, float len,
        float x1, float y1) {
        m_out << m_name << "linear_segment_with_length("
            << x0 << ", " << y0 << ", "
            << len << ", "
            << x1 << ", " << y1 << ")\n";
        return Base::do_linear_segment(x0, y0, x1, y1);
    }

    void do_degenerate_segment(float x0, float y0, float dx0, float dy0,
            float dx1, float dy1, float x1, float y1) {
        m_out << m_name << "degenerate_segment("
            << x0 << ", " << y0 << ", "
            << dx0 << ", " << dy0 << ", "
            << dx1 << ", " << dy1 << ", "
            << x1 << ", " << y1 << ")\n";
        return Base::do_degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1);
    }

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
        m_out << m_name << "quadratic_segment("
            << x0 << ", " << y0 << ", "
            << x1 << ", " << y1 << ", "
            << x2 << ", " << y2 << ")\n";
        return Base::do_quadratic_segment(x0, y0, x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment(float x0, float y0, float x1, float y1,
        float w1, float x2, float y2) {
        m_out << m_name << "rational_quadratic_segment("
            << x0 << ", " << y0 << ", "
            << x1 << ", " << y1 << ", " << w1 << ", "
            << x2 << ", " << y2 << ")\n";
        return Base::do_rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2);
    }

    void do_cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
        m_out << m_name << "cubic_segment("
            << x0 << ", " << y0 << ", "
            << x1 << ", " << y1 << ", "
            << x2 << ", " << y2 << ", "
            << x3 << ", " << y3 << ")\n";
        return Base::do_cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3);
    }

    void do_begin_segment(float ddx, float ddy, float ddw,
            float dx, float dy, float dw, float x, float y) {
        m_out << m_name << "begin_segment("
            << ddx << ", " << ddy << ", " << ddw << ", "
            << dx << ", " << dy << ", " << dw << ", "
            << x << ", " << y << ")\n";
        return Base::do_begin_segment(ddx, ddy, ddw, dx, dy, dw, x, y);
    }

    void do_end_segment(float ddx, float ddy, float ddw,
            float dx, float dy, float dw, float x, float y) {
        m_out << m_name << "end_segment("
            << ddx << ", " << ddy << ", " << ddw << ", "
            << dx << ", " << dy << ", " << dw << ", "
            << x << ", " << y << ")\n";
        return Base::do_end_segment(ddx, ddy, ddw, dx, dy, dw, x, y);
    }

    void do_begin_open_contour(int len, float x0, float y0) {
        m_out << m_name << "begin_open_contour("
            << len << ", " << x0 << ", " << y0 << ")\n";
        return Base::do_begin_open_contour(len, x0, y0);
    }

    void do_end_open_contour(float x0, float y0, int len) {
        m_out << m_name << "end_open_contour("
            << x0 << ", " << y0 << ", " << len << ")\n";
        return Base::do_end_open_contour(x0, y0, len);
    }

    void do_begin_closed_contour(int len, float x0, float y0) {
        m_out << m_name << "begin_closed_contour("
            << len << ", " << x0 << ", " << y0 << ")\n";
        return Base::do_begin_closed_contour(len, x0, y0);
    }

    void do_end_closed_contour(float x0, float y0, int len) {
        m_out << m_name << "end_closed_contour("
            << x0 << ", " << y0 << ", " << len << ")\n";
        return Base::do_end_closed_contour(x0, y0, len);
    }

    void do_begin_dash(int len, float x0, float y0) {
        m_out << m_name << "begin_dash("
            << len << ", " << x0 << ", " << y0 << ")\n";
        return Base::do_begin_dash(len, x0, y0);
    }

    void do_end_dash(float x0, float y0, int len) {
        m_out << m_name << "end_dash("
            << x0 << ", " << y0 << ", " << len << ")\n";
        return Base::do_end_dash(x0, y0, len);
    }

private:
    std::string m_name;
    std::ostream &m_out;
};

template <typename SINK>
Spy<SINK> make_spy(const char *name, SINK &&sink,
std::ostream &out = std::cerr) {
    return Spy<SINK>(name, std::forward<SINK>(sink), out);
}

inline Spy<Null> make_spy(const char *name, std::ostream &out = std::cerr) {
    return Spy<Null>(name, Null(), out);
}

} } } // namespace rvg::path::filter

#endif
