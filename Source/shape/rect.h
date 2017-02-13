#ifndef RVG_SHAPE_RECT_H
#define RVG_SHAPE_RECT_H

#include <memory>

#include "path/path.h"
#include "xform/xform.h"

namespace rvg {
    namespace shape {

class Rect {

float m_x, m_y, m_width, m_height;

public:
    Rect(float x, float y, float width, float height):
        m_x(x), m_y(y), m_width(width), m_height(height) { ; }

    float x(void) const { return m_x; }
    float y(void) const { return m_y; }
    float width(void) const { return m_width; }
    float height(void) const { return m_height; }

    path::PathPtr as_path_ptr(const xform::Xform &post_xf) const {
        (void) post_xf;
        float x2 = m_x + m_width;
        float y2 = m_y + m_height;
        auto p = std::make_shared<path::Path>();
        p->begin_closed_contour(5, m_x, m_y);
        p->linear_segment(m_x, m_y, x2, m_y);
        p->linear_segment(x2, m_y, x2, y2);
        p->linear_segment(x2, y2, m_x, y2);
        p->linear_segment(m_x, y2, m_x, m_y);
        p->end_closed_contour(m_x, m_y, 5);
        return p;
    }
};

using RectPtr = std::shared_ptr<Rect>;

} } // namespace rvg::shape

#endif
