#ifndef RVG_SHAPE_H
#define RVG_SHAPE_H

#include <memory>
#include <cstring>

#include "path/path.h"
#include "xform/xformable.h"
#include "stroke/style.h"
#include "stroke/istrokable.h"
#include "shape/circle.h"
#include "shape/triangle.h"
#include "shape/rect.h"
#include "shape/polygon.h"

namespace rvg {
    namespace shape {

// forward declaration
class Shape;
using ShapePtr = std::shared_ptr<Shape>;

class Shape final: public rvg::xform::Xformable<Shape>,
                   public rvg::stroke::IStrokable<Shape, rvg::stroke::Style> {
private:

    using PathPtr = rvg::path::PathPtr;
    using Path = rvg::path::Path;

    using CirclePtr = rvg::shape::CirclePtr;
    using Circle = rvg::shape::Circle;

    using TrianglePtr = rvg::shape::TrianglePtr;
    using Triangle = rvg::shape::Triangle;

    using PolygonPtr = rvg::shape::PolygonPtr;
    using Polygon = rvg::shape::Polygon;

    using RectPtr = rvg::shape::RectPtr;
    using Rect = rvg::shape::Rect;

    using Style = rvg::stroke::Style;
    using StylePtr = rvg::stroke::StylePtr;

    using Cap = rvg::stroke::Cap;
    using Join = rvg::stroke::Join;
    using Method = rvg::stroke::Method;
    using DashArray = rvg::stroke::DashArray;

public:
    enum class Type {
        path,
        circle,
        triangle,
        rect,
        polygon,
        blend,
        stroke,
        empty,
    };

    class Blend {
        PathPtr m_from_ptr, m_to_ptr;
        float m_t;
    public:
        Blend(const PathPtr &from_ptr,  const PathPtr &to_ptr, float t):
            m_from_ptr(from_ptr), m_to_ptr(to_ptr), m_t(t) { ; }
        const Path &from(void) const { return *m_from_ptr; }
        const Path &to(void) const { return *m_to_ptr; }
        const PathPtr &from_ptr(void) const { return m_from_ptr; }
        const PathPtr &to_ptr(void) const { return m_to_ptr; }
        float t(void) const { return m_t; }
    };

    class Stroke {
        ShapePtr m_shape_ptr;
        StylePtr m_style_ptr;
    public:
        Stroke(const ShapePtr &shape_ptr, const StylePtr &style_ptr):
            m_shape_ptr(shape_ptr), m_style_ptr(style_ptr) { ; }
        const Shape &shape(void) const { return *m_shape_ptr; }
        const Style &style(void) const { return *m_style_ptr; }
        const ShapePtr &shape_ptr(void) const { return m_shape_ptr; }
        const StylePtr &style_ptr(void) const { return m_style_ptr; }
    };

private:

    Type m_type;

    union Union {
        Union() { ; }
        ~Union() { ; }
        PathPtr path_ptr;
        CirclePtr circle_ptr;
        TrianglePtr triangle_ptr;
        RectPtr rect_ptr;
        PolygonPtr polygon_ptr;
        Blend blend;
        Stroke stroke;
    } m_union;

public:

    Shape(): m_type(Type::empty) {
        memset(&m_union, 0, sizeof(m_union));
    }

    Shape(const Shape &other):
        Xformable<Shape>(other),
        m_type(other.m_type) {
        switch (m_type) {
            case Type::path:
                new (&m_union.path_ptr) PathPtr(other.m_union.path_ptr);
                break;
            case Type::circle:
                new (&m_union.circle_ptr) CirclePtr(other.m_union.circle_ptr);
                break;
            case Type::triangle:
                new (&m_union.triangle_ptr) TrianglePtr(other.m_union.triangle_ptr);
                break;
            case Type::rect:
                new (&m_union.rect_ptr) RectPtr(other.m_union.rect_ptr);
                break;
            case Type::polygon:
                new (&m_union.polygon_ptr) PolygonPtr(other.m_union.polygon_ptr);
                break;
            case Type::blend:
                new (&m_union.blend) Blend(other.m_union.
                    blend);
                break;
            case Type::stroke:
                new (&m_union.stroke) Stroke(other.m_union.
                    stroke);
                break;
            case Type::empty:
                memset(&m_union, 0, sizeof(m_union));
                break;
        }
    }

    Shape(Shape &&other):
        Xformable<Shape>(std::move(other)),
		m_type(std::move(other.m_type)) {
        switch (m_type) {
            case Type::path:
                new (&m_union.path_ptr) PathPtr(std::move(other.m_union.
                    path_ptr));
                break;
            case Type::circle:
                new (&m_union.circle_ptr) CirclePtr(std::move(other.m_union.
                    circle_ptr));
                break;
            case Type::triangle:
                new (&m_union.triangle_ptr) TrianglePtr(std::move(other.m_union.
                    triangle_ptr));
                break;
            case Type::rect:
                new (&m_union.rect_ptr) RectPtr(std::move(other.m_union.
                    rect_ptr));
                break;
            case Type::polygon:
                new (&m_union.polygon_ptr) PolygonPtr(std::move(other.m_union.
                    polygon_ptr));
                break;
            case Type::blend:
                new (&m_union.blend) Blend(std::move(other.
                    m_union.blend));
                break;
            case Type::stroke:
                new (&m_union.stroke) Stroke(std::move(other.
                    m_union.stroke));
                break;
            case Type::empty:
                memset(&m_union, 0, sizeof(m_union));
                break;
        }
    }

    explicit Shape(const PathPtr &path_ptr):
        m_type(Type::path) {
        new (&m_union.path_ptr) PathPtr{path_ptr};
    }

    explicit Shape(const CirclePtr &circle_ptr):
        m_type(Type::circle) {
        new (&m_union.circle_ptr) CirclePtr{circle_ptr};
    }

    explicit Shape(const TrianglePtr &triangle_ptr):
        m_type(Type::triangle) {
        new (&m_union.triangle_ptr) TrianglePtr{triangle_ptr};
    }

    explicit Shape(const RectPtr &rect_ptr):
        m_type(Type::rect) {
        new (&m_union.rect_ptr) RectPtr{rect_ptr};
    }

    explicit Shape(const PolygonPtr &polygon_ptr):
        m_type(Type::polygon) {
        new (&m_union.polygon_ptr) PolygonPtr{polygon_ptr};
    }

    Shape(const PathPtr &from, const PathPtr &to, float t):
        m_type(Type::blend) {
        new (&m_union.blend) Blend{from, to, t};
    }

    Shape(const ShapePtr &shape, const StylePtr &style):
        m_type(Type::stroke) {
        new (&m_union.stroke) Stroke{shape, style};
    }

    const char *type_name(void) const {
        switch (m_type) {
            case Type::path: return "path";
            case Type::circle: return "circle";
            case Type::triangle: return "triangle";
            case Type::rect: return "rect";
            case Type::polygon: return "polygon";
            case Type::blend: return "blend";
            case Type::stroke: return "stroke";
            case Type::empty: return "empty";
            default: return "uninitialized?";
        }
    }

    ~Shape() {
        switch (m_type) {
            case Type::path:
                m_union.path_ptr.~PathPtr();
                break;
            case Type::circle:
                m_union.circle_ptr.~CirclePtr();
                break;
            case Type::triangle:
                m_union.triangle_ptr.~TrianglePtr();
                break;
            case Type::rect:
                m_union.rect_ptr.~RectPtr();
                break;
            case Type::polygon:
                m_union.polygon_ptr.~PolygonPtr();
                break;
            case Type::blend:
                m_union.blend.~Blend();
                break;
            case Type::stroke:
                m_union.stroke.~Stroke();
                break;
            case Type::empty:
                break;
        }
    }

    Shape &operator=(const Shape &other) {
        // ??D we should optimize for case where other.m_type == m_type
        // in this case we could avoid destruction and construction
        // and invoke the copy assignment directly in the m_union field
        this->~Shape();
        new (this) Shape(other);
        return *this;
    }

    Shape &operator=(Shape &&other) {
        // ??D we should optimize for case where other.m_type == m_type
        // in this case we could avoid destruction and construction
        // and invoke the move assignment directly in the m_union field
        this->~Shape();
        new (this) Shape(std::move(other));
        return *this;
    }

    Shape as_path_shape(const xform::Xform &post_xf) const {
        xform::Xform pxf = xf().transformed(post_xf);
        PathPtr path_ptr;
        switch (m_type) {
            case Type::path:
                path_ptr = m_union.path_ptr;
                break;
            case Type::circle:
                path_ptr = m_union.circle_ptr->as_path_ptr(pxf);
                break;
            case Type::triangle:
                path_ptr = m_union.triangle_ptr->as_path_ptr(pxf);
                break;
            case Type::rect:
                path_ptr = m_union.rect_ptr->as_path_ptr(pxf);
                break;
            case Type::polygon:
                path_ptr = m_union.polygon_ptr->as_path_ptr(pxf);
                break;
            case Type::blend:
                // ??D not implemented yet
                path_ptr = m_union.blend.from_ptr();
                break;
            case Type::stroke: {
                // ??D not implemented yet
                Shape stroke = m_union.stroke.shape().
                    as_path_shape(pxf);
                return stroke.transformed(xf());
            }
            case Type::empty:
                path_ptr = std::make_shared<Path>();
                break;
        }
        return Shape(path_ptr).transformed(xf());
    }

    const Type &type(void) const
    { return m_type; }

    const Blend &blend(void) const
    { return m_union.blend; }

    const Stroke &stroke(void) const
    { return m_union.stroke; }

    const PathPtr &path_ptr(void) const
    { return m_union.path_ptr; }
    const Path &path(void) const
    { return *m_union.path_ptr; }

    const CirclePtr &circle_ptr(void) const
    { return m_union.circle_ptr; }
    const Circle &circle(void) const
    { return *m_union.circle_ptr; }

    const TrianglePtr &triangle_ptr(void) const
    { return m_union.triangle_ptr; }
    const Triangle &triangle(void) const
    { return *m_union.triangle_ptr; }

    const RectPtr &rect_ptr(void) const
    { return m_union.rect_ptr; }
    const Rect &rect(void) const
    { return *m_union.rect_ptr; }

    const PolygonPtr &polygon_ptr(void) const
    { return m_union.polygon_ptr; }
    const Polygon &polygon(void) const
    { return *m_union.polygon_ptr; }

private: // IStrokable interface

    friend IStrokable<Shape, rvg::stroke::Style>;

    Shape do_stroked(float width) const {
        // allocate a brand new style in heap
        // return a stroked version of this shape
        return Shape{
            std::make_shared<Shape>(*this),
            std::make_shared<Style>(Style().stroked(width))
        };
    }

    Shape do_stroked(const Style &style) const {
        // allocate a brand new style in heap
        // return a stroked version of this shape
        return Shape{
            std::make_shared<Shape>(*this),
            std::make_shared<Style>(style)
        };
    }

    Shape do_dashed(const DashArray &dash_array) const {
		// if current shape is not stroked, ignore
		if (m_type == Type::stroke) {
			return Shape{
				stroke().shape_ptr(),
				std::make_shared<Style>(stroke().style().
                    dashed(dash_array))
			};
		} else {
            return *this;
        }
    }

    Shape do_dashed(const DashArray &dash_array, float initial_phase,
        bool phase_reset) const {
		// if current shape is not stroked, ignore
		if (m_type == Type::stroke) {
			return Shape{
				stroke().shape_ptr(),
				std::make_shared<Style>(stroke().style().
                    dashed(dash_array, initial_phase, phase_reset))
			};
		} else {
            return *this;
        }
    }

    Shape do_capped(Cap cap) const {
		// if current shape is not stroked, ignore
		if (m_type == Type::stroke) {
			return Shape{
				stroke().shape_ptr(),
				std::make_shared<Style>(stroke().style().capped(cap))
			};
		} else {
            return *this;
        }
    }

    Shape do_joined(Join join) const {
		// if current shape is not stroked, ignore
		if (m_type == Type::stroke) {
			return Shape{
				stroke().shape_ptr(),
				std::make_shared<Style>(stroke().style().joined(join))
			};
		} else {
            return *this;
        }
    }

    Shape do_joined(Join join, float miter_limit) const {
		// if current shape is not stroked, ignore
		if (m_type == Type::stroke) {
			return Shape{
				stroke().shape_ptr(),
				std::make_shared<Style>(
					stroke().style().joined(join, miter_limit))
			};
		} else {
            return *this;
        }
    }

    Shape do_by(Method method) const {
		// if current shape is not stroked, ignore
		if (m_type == Type::stroke) {
			return Shape{
				stroke().shape_ptr(),
				std::make_shared<Style>(stroke().style().by(method))
			};
		} else {
            return *this;
        }
    }
};

} } // namespace rvg::shape

#endif
