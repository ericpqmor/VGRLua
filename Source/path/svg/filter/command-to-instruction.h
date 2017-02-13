#ifndef RVG_PATH_SVG_FILTER_COMMAND_TO_INSTRUCTION_H
#define RVG_PATH_SVG_FILTER_COMMAND_TO_INSTRUCTION_H

#include <utility>

#include "path/ipath.h"
#include "path/svg/isvgpath.h"
#include "point/point.h"
#include "xform/xform.h"
#include "math/math.h"

namespace rvg {
    namespace path {
        namespace svg {
            namespace filter {

template <typename SINK>
class CommandToInstruction final: public ISVGPath<CommandToInstruction<SINK>> {

    SINK m_sink;
    float m_current_x, m_current_y;
    float m_previous_x, m_previous_y;
    float m_start_x, m_start_y;
    bool m_begun;

public:

    explicit CommandToInstruction(SINK &&sink):
        m_sink(std::forward<SINK>(sink)) {
        static_assert(rvg::meta::is_an_ipath<SINK>::value,
            "sink is not an IPath");
		this->reset();
	}

    void end(void) {
        ensure_ended();
    }

private:

    friend ISVGPath<CommandToInstruction<SINK>>;

    void do_move_to_abs(float x0, float y0) {
        ensure_ended();
        m_sink.begin_open_contour(0, x0, y0);
        m_begun = true;
        set_start(x0, y0);
        set_current(x0, y0);
        set_previous(x0, y0);
    }

    void do_move_to_rel(float x0, float y0) {
        x0 += m_current_x;
        y0 += m_current_y;
        this->move_to_abs(x0, y0);
    }

    void do_close_path(void) {
        ensure_begun();
        m_sink.end_closed_contour(m_current_x, m_current_y, 0);
        m_begun = false;
    }

    void do_line_to_abs(float x1, float y1) {
        ensure_begun();
        m_sink.linear_segment(m_current_x, m_current_y, x1, y1);
        set_current(x1, y1);
        set_previous(x1, y1);
    }

    void do_line_to_rel(float x1, float y1) {
        x1 += m_current_x;
        y1 += m_current_y;
        this->line_to_abs(x1, y1);
    }

    void do_hline_to_abs(float x1) {
        this->line_to_abs(x1, m_current_y);
    }

    void do_hline_to_rel(float x1) {
        x1 += m_current_x;
        this->hline_to_abs(x1);
    }

    void do_vline_to_abs(float y1) {
        this->line_to_abs(m_current_x, y1);
    }

    void do_vline_to_rel(float y1) {
        y1 += m_current_y;
        this->vline_to_abs(y1);
    }

    void do_quad_to_abs(float x1, float y1, float x2, float y2) {
        ensure_begun();
        m_sink.quadratic_segment(m_current_x, m_current_y, x1, y1, x2, y2);
        set_previous(x1, y1);
        set_current(x2, y2);
    }

    void do_quad_to_rel(float x1, float y1, float x2, float y2) {
        x1 += m_current_x;
        y1 += m_current_y;
        x2 += m_current_x;
        y2 += m_current_y;
        this->quad_to_abs(x1, y1, x2, y2);
    }

    void do_squad_to_abs(float x2, float y2) {
        float x1 = 2.f*m_current_x - m_previous_x;
        float y1 = 2.f*m_current_y - m_previous_y;
        this->quad_to_abs(x1, y1, x2, y2);
    }

    void do_squad_to_rel(float x2, float y2) {
        x2 += m_current_x;
        y2 += m_current_y;
        this->squad_to_abs(x2, y2);
    }

    void do_rquad_to_abs(float x1, float y1, float w1, float x2, float y2) {
        ensure_begun();
        m_sink.rational_quadratic_segment(m_current_x, m_current_y,
            x1, y1, w1, x2, y2);
        set_previous(x2, y2);
        set_current(x2, y2);
    }

    void do_rquad_to_rel(float x1, float y1, float w1, float x2, float y2) {
        x1 += m_current_x*w1;
        y1 += m_current_y*w1;
        x2 += m_current_x;
        y2 += m_current_y;
        this->rquad_to_abs(x1, y1, w1, x2, y2);
    }

    void do_svgarc_to_abs(float rx, float ry, float rot_deg,
        float ffa, float ffs, float x2, float y2) {
        using namespace rvg::math;
        using namespace rvg::point;
        using namespace rvg::xform;
        bool fa = (ffa != 0.f), fs = (ffs != 0.f);
        float x0 = m_current_x, y0 = m_current_y;
        // radii are assumed positive
        rx = std::abs(rx); ry = std::abs(ry);
        // if radii are too small, we degenerate to line connecting endpoints
        if (is_almost_zero(rx) || is_almost_zero(ry)) {
            return m_sink.linear_segment(x0, y0, x2, y2);
        }
        float rot_rad = rad(rot_deg);
        float cos_rot = std::cos(rot_rad);
        float sin_rot = std::sin(rot_rad);
        R2 p0{x0, y0}, p2{x2, y2};
        Scaling S{1.f/rx, 1.f/ry};
        Rotation R{cos_rot, -sin_rot};
        // we solve the problem in a new coordinate system
        // where rx=ry=1 and rot_deg=0, then we move the solution
        // back to the original coordinate system
        R2 q0 = S.apply(R.apply(p0));
        R2 q2 = S.apply(R.apply(p2));
        // direction perpendicular to line connecting endpoints
        R2 q20p = perp(q2-q0);
        // if transformed endpoints are too close, degenerate to
        // line segment connecting endpoints
        float el2 = len2(q20p); // perp doesn't change length
        if (is_almost_zero(el2)) {
            return m_sink.linear_segment(x0, y0, x2, y2);
        }
        R2 mq = .5f*(q0+q2); // midpoint between transformed endpoints
        float radius; // circle radius
        float inv_radius; // its reciprocal
        float offset; // distance from midpoint to center
        // center of circle, endpoint, and midpoint form a right triangle
        // hypotenuse is the circle radius, which has length 1
        // it connects the endpoint to the center
        // the segment connecting the midpoint and endpoint is a cathetus
        // the segment connecting midpoint and the center is the other cathetus
        float el = std::sqrt(el2);
        float inv_el = 1.f/el;
        // the length of the hypothenuse must be at least
        // as large as the length of the catheti.
        if (el2 > 4.f) {
            // otherwise, we grow the circle isotropically until they are equal
            radius = .5f*el;
            inv_radius = 2.f*inv_el;
            // in which case, the midpoint *is* the center
            offset = 0.f;
        } else {
            // circle with radius 1 is large enough
            radius = 1.f;
            inv_radius = 1.f;
            // length of the cathetus connecting the midpoint and the center
            offset = .5f*std::sqrt(4.f-el2);
        }
        // there are two possible circles. flags decide which one
        float sign = (fa != fs)? 1.f : -1.f; // offset sign
        // to find circle center in new coordinate system,
        // simply offset midpoint in the perpendicular direction
        R2 cq = mq + (sign*offset*inv_el)*q20p;
        // middle weight is the cosine of half the sector angle
        float w1 = std::abs(dot(q0-cq, q20p)*inv_el*inv_radius);
        // if center was at the origin, this would be the
        // intermediate control point for the rational quadratic
        // so we translate it by the center cq
        RP2 q1 = RP2{(-sign*radius*inv_el)*q20p, w1} + RP2{cq};
        // move control point back to original coordinate system
        Scaling iS{rx, ry};
        Rotation iR{cos_rot, sin_rot};
        q1 = iR.apply(iS.apply(q1));
        // this selects the small arc. to select the large arc,
        // negate all coordinates of intermediate control point
        // ??D this is not a good idea. We should instead
        // split the arc into pieces that subintend at most 2*Pi/3 radians
        if (fa) {
            m_sink.rational_quadratic_segment(x0, y0, -q1[0], -q1[1], -q1[2],
                x2, y2);
        } else {
            m_sink.rational_quadratic_segment(x0, y0, q1[0], q1[1], q1[2],
                x2, y2);
        }
        set_previous(x2, y2);
        set_current(x2, y2);
    }

    void do_svgarc_to_rel(float rx, float ry, float a, float fa, float fs,
        float x2, float y2) {
        x2 += m_current_x;
        y2 += m_current_y;
        this->svgarc_to_abs(rx, ry, a, fa, fs, x2, y2);
    }

    void do_cubic_to_abs(float x1, float y1, float x2, float y2,
        float x3, float y3) {
        ensure_begun();
        m_sink.cubic_segment(m_current_x, m_current_y, x1, y1, x2, y2, x3, y3);
        set_previous(x2, y2);
        set_current(x3, y3);
    }

    void do_cubic_to_rel(float x1, float y1, float x2, float y2,
        float x3, float y3) {
        x1 += m_current_x;
        y1 += m_current_y;
        x2 += m_current_x;
        y2 += m_current_y;
        x3 += m_current_x;
        y3 += m_current_y;
        this->cubic_to_abs(x1, y1, x2, y2, x3, y3);
    }

    void do_scubic_to_abs(float x2, float y2, float x3, float y3) {
        float x1 = 2.f*m_current_x - m_previous_x;
        float y1 = 2.f*m_current_y - m_previous_y;
        this->cubic_to_abs(x1, y1, x2, y2, x3, y3);
    }

    void do_scubic_to_rel(float x2, float y2, float x3, float y3) {
        x2 += m_current_x;
        y2 += m_current_y;
        x3 += m_current_x;
        y3 += m_current_y;
        this->scubic_to_abs(x2, y2, x3, y3);
    }

    void reset(void) {
        m_begun = false;
        set_start(0.f, 0.f);
        set_current(0.f, 0.f);
        set_previous(0.f, 0.f);
    }

    void set_start(float x, float y) {
        m_start_x = x;
        m_start_y = y;
    }

    void set_current(float x, float y) {
        m_current_x = x;
        m_current_y = y;
    }

    void set_previous(float x, float y) {
        m_previous_x = x;
        m_previous_y = y;
    }

    void ensure_begun(void) {
        if (!m_begun) {
            m_sink.begin_open_contour(0, m_current_x, m_current_y);
            m_begun = true;
        }
    }

    void ensure_ended(void) {
        if (m_begun) {
            m_sink.end_open_contour(m_current_x, m_current_y, 0);
            m_begun = false;
        }
    }
};

template <typename SINK>
CommandToInstruction<SINK> make_command_to_instruction(SINK &&sink) {
    return CommandToInstruction<SINK>(std::forward<SINK>(sink));
}

} } } } // namespace rvg::path::svg::filter

#endif
