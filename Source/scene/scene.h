#ifndef RVG_SCENE_H
#define RVG_SCENE_H

#include <cstdint>
#include <iostream>

#include "scene/iscene.h"
#include "xform/xform.h"

// ??D Expose some way for array of paints and shapes to be modified?
namespace rvg {
    namespace scene {

class Scene final: public IScene<Scene> {

    struct Element {
        enum class Type: uint8_t {
            painted,  // element is filled with paint
            stencil   // element is a pure shape used for clipping
        };
        Type m_type;
        WindingRule m_winding_rule;
        ShapeId m_shape_id;     // geometry for painted and stencils
        PaintId m_paint_id;     // paint for painted, unused for stencils

        Element(const Type &type, const WindingRule &winding_rule,
            const ShapeId &shape_id, const PaintId &paint_id =
                std::numeric_limits<PaintId>::max()):
            m_type(type),
            m_winding_rule(winding_rule),
            m_shape_id(shape_id),
            m_paint_id(paint_id)
            { ; }
    };

    struct Bracket {
        enum class Type: uint8_t {
            begin_clip,
            activate_clip,
            end_clip,
            begin_fade,
            end_fade,
            begin_blur,
            end_blur,
            begin_transform,
            end_transform
        };
        Type m_type;
        uint16_t m_depth;       // nesting depth
        ElementId m_element_id; // index in element array where bracket lives

        // ??D These bracket-specific data could be inside of an union.
        // Also, should probably reorder them to take less space.
        uint8_t m_opacity;      // opacity for fade group
        float m_blur_radius;    // standard deviation for blur group
        xform::Xform m_xf;      // xform for transform group

        Bracket(const Type &type, const ElementId &element_id, uint16_t depth):
            m_type(type),
            m_depth(depth),
            m_element_id(element_id),
            m_opacity(255),
            m_blur_radius(0.f),
            m_xf(xform::Identity{})
            { ; }

        Bracket(const Type &type, const ElementId &element_id, uint16_t depth,
            uint8_t opacity):
            m_type(type),
            m_depth(depth),
            m_element_id(element_id),
            m_opacity(opacity),
            m_blur_radius(0.f),
            m_xf(xform::Identity{})
            { ; }

        Bracket(const Type &type, const ElementId &element_id,
            uint16_t depth, const xform::Xform &xf):
            m_type(type),
            m_depth(depth),
            m_element_id(element_id),
            m_opacity(255),
            m_blur_radius(0.f),
            m_xf(xf)
            { ; }

        Bracket(const Type &type, const ElementId &element_id, uint16_t depth,
            float blur_radius):
            m_type(type),
            m_depth(depth),
            m_element_id(element_id),
            m_opacity(255),
            m_blur_radius(blur_radius),
            m_xf(xform::Identity{})
            { ; }
    };

public:

    template <typename SF>
    void iterate(SF &forward) const {
        std::size_t bracket_index = 0, element_index = 0;
        while (element_index < m_elements.size() ||
               bracket_index < m_brackets.size()) {
            if (bracket_index < m_brackets.size() &&
                m_brackets[bracket_index].m_element_id <= element_index) {
                const auto &b = m_brackets[bracket_index];
                using BT = Bracket::Type;
                switch (b.m_type) {
                    case BT::begin_clip:
                        forward.begin_clip(b.m_depth);
                        break;
                    case BT::activate_clip:
                        forward.activate_clip(b.m_depth);
                        break;
                    case BT::end_clip:
                        forward.end_clip(b.m_depth);
                        break;
                    case BT::begin_fade:
                        forward.begin_fade(b.m_depth, b.m_opacity);
                        break;
                    case BT::end_fade:
                        forward.end_fade(b.m_depth, b.m_opacity);
                        break;
                    case BT::begin_blur:
                        forward.begin_blur(b.m_depth, b.m_blur_radius);
                        break;
                    case BT::end_blur:
                        forward.end_blur(b.m_depth, b.m_blur_radius);
                        break;
                    case BT::begin_transform:
                        forward.begin_transform(b.m_depth, b.m_xf);
                        break;
                    case BT::end_transform:
                        forward.end_transform(b.m_depth, b.m_xf);
                        break;
                }
                ++bracket_index;
            } else if (element_index < m_elements.size()) {
                const auto &e = m_elements[element_index];
                using ET = Element::Type;
                switch (e.m_type) {
                    case ET::painted:
                        forward.painted_element(e.m_winding_rule,
                            m_shapes[e.m_shape_id],
                            m_paints[e.m_paint_id]);
                        break;
                    case ET::stencil:
                        forward.stencil_element(e.m_winding_rule,
                            m_shapes[e.m_shape_id]);
                        break;
                }
                ++element_index;
            } else {
                break;
            }
        }
    }

    const std::vector<Shape> &shapes(void) const {
        return m_shapes;
    }

    const std::vector<Paint> &paints(void) const {
        return m_paints;
    }

private:

    std::vector<Shape> m_shapes;
    std::vector<Paint> m_paints;
    std::vector<Bracket> m_brackets;
    std::vector<Element> m_elements;

    void push_clip_bracket(const Bracket::Type &type, uint16_t depth) {
        m_brackets.emplace_back(type, m_elements.size(), depth);
    }

    void push_fade_bracket(const Bracket::Type &type, uint16_t depth,
        uint8_t opacity) {
        m_brackets.emplace_back(type, m_elements.size(), depth, opacity);
    }

    void push_blur_bracket(const Bracket::Type &type,
        uint16_t depth, float radius) {
        m_brackets.emplace_back(type, m_elements.size(), depth, radius);
    }

    void push_transform_bracket(const Bracket::Type &type,
        uint16_t depth, const xform::Xform &xf) {
        m_brackets.emplace_back(type, m_elements.size(), depth, xf);
    }

    friend IScene<Scene>;

    PaintId do_push_paint(const Paint &paint) {
        PaintId pid(m_paints.size());
        m_paints.push_back(paint);
        return pid;
    }

    ShapeId do_push_shape(const Shape &shape) {
        ShapeId pid(m_shapes.size());
        m_shapes.push_back(shape);
        return pid;
    }

    void do_painted_element(WindingRule winding_rule, ShapeId shape_id,
        PaintId paint_id) {
        m_elements.emplace_back(Element::Type::painted, winding_rule, shape_id,
            paint_id);
    }

    void do_painted_element(WindingRule winding_rule, const Shape &shape,
        const Paint &paint) {
        m_elements.emplace_back(Element::Type::painted, winding_rule,
            push_shape(shape), push_paint(paint));
    }

    void do_stencil_element(WindingRule winding_rule, ShapeId shape_id) {
        m_elements.emplace_back(Element::Type::stencil, winding_rule, shape_id);
    }

    void do_stencil_element(WindingRule winding_rule, const Shape &shape) {
        m_elements.emplace_back(Element::Type::stencil, winding_rule,
            push_shape(shape));
    }

    void do_begin_clip(uint16_t depth) {
        push_clip_bracket(Bracket::Type::begin_clip, depth);
    }

    void do_activate_clip(uint16_t depth) {
        push_clip_bracket(Bracket::Type::activate_clip, depth);
    }

    void do_end_clip(uint16_t depth) {
        push_clip_bracket(Bracket::Type::end_clip, depth);
    }

    void do_begin_fade(uint16_t depth, uint8_t opacity) {
        push_fade_bracket(Bracket::Type::begin_fade, depth, opacity);
    }

    void do_end_fade(uint16_t depth, uint8_t opacity) {
        push_fade_bracket(Bracket::Type::end_fade, depth, opacity);
    }

    void do_begin_blur(uint16_t depth, float radius) {
        push_blur_bracket(Bracket::Type::begin_blur, depth, radius);
    }

    void do_end_blur(uint16_t depth, float radius) {
        push_blur_bracket(Bracket::Type::end_blur, depth, radius);
    }

    void do_begin_transform(uint16_t depth, const xform::Xform &xf) {
        push_transform_bracket(Bracket::Type::begin_transform, depth, xf);
    }

    void do_end_transform(uint16_t depth, const xform::Xform &xf) {
        push_transform_bracket(Bracket::Type::end_transform, depth, xf);
    }
};

using ScenePtr = std::shared_ptr<Scene>;

} } // namespace rvg::scene

#endif
