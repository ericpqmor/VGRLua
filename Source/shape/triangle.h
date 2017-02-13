#ifndef RVG_SHAPE_TRIANGLE_H
#define RVG_SHAPE_TRIANGLE_H

#include <memory>

#include "path/path.h"
#include "xform/xform.h"

namespace rvg {
    namespace shape {

class Triangle {

float m_x1, m_y1, m_x2, m_y2, m_x3, m_y3;

public:
    Triangle(float x1, float y1, float x2, float y2, float x3, float y3):
        m_x1(x1), m_y1(y1), m_x2(x2), m_y2(y2), m_x3(x3), m_y3(y3) { ; }

    float x1(void) const { return m_x1; }
    float y1(void) const { return m_y1; }
    float x2(void) const { return m_x2; }
    float y2(void) const { return m_y2; }
    float x3(void) const { return m_x3; }
    float y3(void) const { return m_y3; }

    path::PathPtr as_path_ptr(const xform::Xform &post_xf) const {
        (void) post_xf;
        auto p = std::make_shared<path::Path>();
        p->begin_closed_contour(4, m_x1, m_y1);
        p->linear_segment(m_x1, m_y1, m_x2, m_y2);
        p->linear_segment(m_x2, m_y2, m_x3, m_y3);
        p->linear_segment(m_x3, m_y3, m_x1, m_y1);
        p->end_closed_contour(m_x1, m_y1, 4);
        return p;
    }
};

using TrianglePtr = std::shared_ptr<Triangle>;

} } // namespace rvg::shape

#endif
