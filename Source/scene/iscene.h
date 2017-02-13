#ifndef RVG_SCENE_ISCENE_H
#define RVG_SCENE_ISCENE_H

#include <cstdint>

#include "paint/paint.h"
#include "shape/shape.h"
#include "xform/xform.h"

namespace rvg {
    namespace scene {

enum class WindingRule: uint8_t {
    non_zero,            // non-zero winding number is in
    odd,                 // odd winding number is in
    zero,                // zero winding number is in (non-standard)
    even,                // even winding number is in (non-standard)
};

template <typename DERIVED>
class IScene {
protected:
    using Paint = rvg::paint::Paint;
    using Shape = rvg::shape::Shape;
    using Sform = rvg::xform::Xform;
    using WindingRule = rvg::scene::WindingRule;

    using ShapeId = std::size_t;
    using PaintId = std::size_t;
    using ElementId = std::size_t;

    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:

    // allocate a new shape
    ShapeId push_shape(const Shape &shape) {
        return derived().do_push_shape(shape);
    }

    // allocate a new paint
    PaintId push_paint(const Paint &paint) {
        return derived().do_push_paint(paint);
    }

    // allocate a new element that is shape filled with paint
    void painted_element(WindingRule rule,
        ShapeId shape_id, PaintId paint_id) {
        return derived().do_painted_element(rule, shape_id, paint_id);
    }

    void painted_element(WindingRule rule,
        const Shape &shape, const Paint &paint) {
        return derived().do_painted_element(rule, shape, paint);
    }

    // allocate a new element that is a pure shape used for clipping
    void stencil_element(WindingRule rule, ShapeId shape_id) {
        return derived().do_stencil_element(rule, shape_id);
    }

    void stencil_element(WindingRule rule, const Shape &shape) {
        return derived().do_stencil_element(rule, shape);
    }

    // clip and transparency group control
    void begin_clip(uint16_t depth) {
        return derived().do_begin_clip(depth);
    }

    void activate_clip(uint16_t depth) {
        return derived().do_activate_clip(depth);
    }

    void end_clip(uint16_t depth) {
        return derived().do_end_clip(depth);
    }

    void begin_fade(uint16_t depth, uint8_t opacity) {
        return derived().do_begin_fade(depth, opacity);
    }

    void end_fade(uint16_t depth, uint8_t opacity) {
        return derived().do_end_fade(depth, opacity);
    }

    void begin_blur(uint16_t depth, float radius) {
        return derived().do_begin_blur(depth, radius);
    }

    void end_blur(uint16_t depth, float radius) {
        return derived().do_end_blur(depth, radius);
    }

    void begin_transform(uint16_t depth, const xform::Xform  &xf) {
        return derived().do_begin_transform(depth, xf);
    }

    void end_transform(uint16_t depth, const xform::Xform &xf) {
        return derived().do_end_transform(depth, xf);
    }

};

} } // namespace rvg::scene

#endif
