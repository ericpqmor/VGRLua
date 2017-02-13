#ifndef RVG_SHAPE_CIRCLE_H
#define RVG_SHAPE_CIRCLE_H

#include <memory>

#include "path/path.h"
#include "xform/xform.h"

namespace rvg {
    namespace shape {

class Circle {

float m_cx, m_cy, m_r;

public:
    Circle(float cx, float cy, float r): m_cx(cx), m_cy(cy), m_r(r) { ; }

    float cx(void) const { return m_cx; }

    float cy(void) const { return m_cy; }

    float r(void) const { return m_r; }

    path::PathPtr as_path_ptr(const xform::Xform &post_xf) const {
        (void) post_xf;
        // we start with a unit circle centered at the origin
        // it is formed by 3 arcs covering each third of the unit circle
        // we then scale it by r and translate it by cx,cy
        static constexpr float s = 0.5f;           // sin(pi/6)
        static constexpr float c = 0.86602540378f; // cos(pi/6)
        static constexpr float w = s;
        auto p = std::make_shared<path::Path>();
        const float x1 = m_cx,          y1 = m_r+m_cy;
        const float x2 = -c*m_r+m_cx*w, y2 = s*m_r+m_cy*w, w2 = w;
        const float x3 = -c*m_r+m_cx,   y3 = -s*m_r+m_cy;
        const float x4 = m_cx*w,        y4 = -m_r+m_cy*w,  w4 = w;
        const float x5 = c*m_r+m_cx,    y5 = -s*m_r+m_cy;
        const float x6 = c*m_r+m_cx*w,  y6 = s*m_r+m_cy*w, w6 = w;
        p->begin_closed_contour(4, x1, y1);
        p->rational_quadratic_segment(x1, y1, x2, y2, w2, x3, y3);
        p->rational_quadratic_segment(x3, y3, x4, y4, w4, x5, y5);
        p->rational_quadratic_segment(x5, y5, x6, y6, w6, x1, y1);
        p->end_closed_contour(x1, y1, 4);
        return p;
    }
};

using CirclePtr = std::shared_ptr<Circle>;

} } // namespace rvg::shape

#endif
