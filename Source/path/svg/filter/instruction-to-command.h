#ifndef RVG_PATH_SVG_FILTER_INSTRUCTION_TO_COMMAND_H
#define RVG_PATH_SVG_FILTER_INSTRUCTION_TO_COMMAND_H

#include <type_traits>

#include "path/ipath.h"
#include "xform/xform.h"
#include "xform/svd.h"

namespace rvg {
    namespace path {
        namespace svg {
            namespace filter {

template <typename SINK>
class InstructionToCommand final: public IPath<InstructionToCommand<SINK>> {

    SINK m_sink;

public:
    // constructor by r-value reference is enabled only
    // if SINK is not a reference type
    explicit InstructionToCommand(SINK &&sink):
    m_sink(std::forward<SINK>(sink)) {
        static_assert(rvg::meta::is_an_isvgpath<SINK>::value,
            "sink is not an ISVGPath");
    }

private:

    friend IPath<InstructionToCommand<SINK>>;

    void do_linear_segment(float x0, float y0, float x1, float y1) {
        (void) x0; (void) y0;
        return m_sink.line_to_abs(x1, y1);
    }

    void do_linear_segment_with_length(float x0, float y0, float len,
        float x1, float y1) {
        (void) x0; (void) y0; (void) len;
        return m_sink.line_to_abs(x1, y1);
    }

    void do_degenerate_segment(float x0, float y0, float dx0, float dy0,
            float dx1, float dy1, float x1, float y1) {
        (void) x0; (void) y0;
        (void) dx0; (void) dy0;
        (void) dx1; (void) dy1;
        return m_sink.line_to_abs(x1, y1);
    }

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
        (void) x0; (void) y0;
        return m_sink.quad_to_abs(x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment(float x0, float y0,
        float x1, float y1, float w1, float x2, float y2) {
        using namespace rvg::xform;
        using namespace rvg::math;
        // we start by computing the projective transformation that
        // maps the unit circle to the ellipse described by the control points
        float s2 = 1.f-w1*w1;
        if (is_almost_zero(s2)) {
            if (is_almost_equal(x0, x1) && is_almost_equal(x1, x2) &&
               is_almost_equal(y0, y1) && is_almost_equal(y1, y2)) {
                // degenerate to a single point?
                return m_sink.line_to_abs(x2, y2);
            } else {
                // degenerate to a parabola?
                return m_sink.quad_to_abs(x1, y1, x2, y2);
            }
        }
        float s = (s2 < 0.f? -1.f: 1.f)*std::sqrt(std::abs(s2));
        // this is the linear part of the transformation;
        Linear L{2.f*x1-w1*(x0+x2), s*(x2-x0), 2.f*y1-w1*(y0+y2), s*(y2-y0)};
        // from it, we get the SVD
        Rotation U; Scaling S;
        svd(L, U, S);
        // the sign of the middle weight gives the large/small angle flag
        float fa = (w1 < 0.f)? 1.f: 0.f;
        // the sign of the area of the control point triangle gives the orientation
        Projectivity T{x0, y0, 1., x1, y1, w1, x2, y2, 1.};
        float fs = det(T) > 0.f? 1.f: 0.f;
        // the rotation and the scaling parts from SVD give the angle and axes
        return m_sink.svgarc_to_abs(S.sx()/(2.f*s2), S.sy()/(2.f*s2),
            deg(std::atan2(U.sin(), U.cos())), fa, fs, x2, y2);
    }

    void do_cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
        (void) x0; (void) y0;
        return m_sink.cubic_to_abs(x1, y1, x2, y2, x3, y3);
    }

    void do_begin_segment(float ddx, float ddy, float ddw,
            float dx, float dy, float dw, float x, float y) {
        (void) x; (void) y;
        (void) dx; (void) dy; (void) dw;
        (void) ddx; (void) ddy; (void) ddw;
    }

    void do_end_segment(float ddx, float ddy, float ddw,
            float dx, float dy, float dw, float x, float y) {
        (void) x; (void) y;
        (void) dx; (void) dy; (void) dw;
        (void) ddx; (void) ddy; (void) ddw;
    }

    void do_begin_open_contour(int len, float x0, float y0) {
        (void) len;
        return m_sink.move_to_abs(x0, y0);
    }

    void do_end_open_contour(float x0, float y0, int len) {
        (void) x0; (void) y0;
        (void) len;
    }

    void do_begin_closed_contour(int len, float x0, float y0) {
        (void) len;
        return m_sink.move_to_abs(x0, y0);
    }

    void do_end_closed_contour(float x0, float y0, int len) {
        (void) x0; (void) y0;
        (void) len;
        return m_sink.close_path();
    }

    void do_begin_dash(int len, float x0, float y0) {
        (void) x0; (void) y0;
        (void) len;
    }

    void do_end_dash(float x0, float y0, int len) {
        (void) x0; (void) y0;
        (void) len;
    }

    uint32_t do_get_data_size(void) const { return 0; }

    uint32_t do_get_instructions_size(void) const { return 0; }

    uint32_t do_get_offsets_size(void) const { return 0; }

    uint32_t do_get_offset(uint32_t index) const {
        (void) index;
        return 0;
    }

    void do_set_datum(uint32_t offset, const Datum &datum) {
        (void) offset; (void) datum;
    }

    void do_set_instruction(uint32_t index,
        const Instruction &instruction) {
        (void) index; (void) instruction;
    }
};

template <typename SINK>
InstructionToCommand<SINK> make_instruction_to_command(SINK &&sink) {
    return InstructionToCommand<SINK>(std::forward<SINK>(sink));
}

} } } } // namespace rvg::path::svg::filter

#endif
