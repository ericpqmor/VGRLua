#ifndef RVG_PAINT_H
#define RVG_PAINT_H

#include <memory>
#include <vector>

#include "color/color.h"
#include "xform/xformable.h"

#include "paint/lineargradient.h"
#include "paint/radialgradient.h"
#include "paint/texture.h"

namespace rvg {
    namespace paint {

class Paint final: public rvg::xform::Xformable<Paint> {
public:

    using Base = rvg::xform::Xformable<Paint>;

    enum class Type {
        solid_color,
        linear_gradient,
        radial_gradient,
        texture
    };

private:
    Type m_type;

    uint8_t m_opacity;

    using SolidColor = rvg::color::RGBA8;

    union Union {
        Union() { ; }
        ~Union() { ; }
        SolidColor solid_color;
        LinearGradientPtr linear_gradient_ptr;
        RadialGradientPtr radial_gradient_ptr;
        TexturePtr texture_ptr;
    } m_union;

public:

    ~Paint() {
        switch (m_type) {
            case Type::solid_color:
                m_union.solid_color.~SolidColor();
                break;
            case Type::linear_gradient:
                m_union.linear_gradient_ptr.~LinearGradientPtr();
                break;
            case Type::radial_gradient:
                m_union.radial_gradient_ptr.~RadialGradientPtr();
                break;
            case Type::texture:
                m_union.texture_ptr.~TexturePtr();
                break;
        }
    }

    Paint() = delete;

    Paint(const Paint &other):
    Base(other), m_type(other.m_type), m_opacity(other.m_opacity) {
        switch (m_type) {
            case Type::solid_color:
                new (&m_union.solid_color) SolidColor(other.m_union.solid_color);
                break;
            case Type::linear_gradient:
                new (&m_union.linear_gradient_ptr)
                    LinearGradientPtr(other.m_union.linear_gradient_ptr);
                break;
            case Type::radial_gradient:
                new (&m_union.radial_gradient_ptr)
                    RadialGradientPtr(other.m_union.radial_gradient_ptr);
                break;
            case Type::texture:
                new (&m_union.texture_ptr)
                    TexturePtr(other.m_union.texture_ptr);
                break;
        }
    }

    Paint(Paint &&other):
        Base(std::move(other)), m_type(std::move(other.m_type)),
        m_opacity(std::move(other.m_opacity)) {
        switch (m_type) {
            case Type::solid_color:
                new (&m_union.solid_color)
                    SolidColor(std::move(other.m_union.solid_color));
                break;
            case Type::linear_gradient:
                new (&m_union.linear_gradient_ptr)
                    LinearGradientPtr(std::move(other.m_union.linear_gradient_ptr));
                break;
            case Type::radial_gradient:
                new (&m_union.radial_gradient_ptr)
                    RadialGradientPtr(std::move(other.m_union.radial_gradient_ptr));
                break;
            case Type::texture:
                new (&m_union.texture_ptr)
                    TexturePtr(std::move(other.m_union.texture_ptr));
                break;
        }
    }

    Paint(const SolidColor &solid_color, int opacity = 255):
        m_type(Type::solid_color),
        m_opacity(rvg::color::int_to_uint8_t(opacity)) {
        new (&m_union.solid_color) SolidColor(solid_color);
    }

    Paint(const LinearGradientPtr &linear_gradient_ptr, int opacity = 255):
        m_type(Type::linear_gradient),
        m_opacity(rvg::color::int_to_uint8_t(opacity)) {
        new (&m_union.linear_gradient_ptr)
            LinearGradientPtr(linear_gradient_ptr);
    }

    Paint(const RadialGradientPtr &radial_gradient_ptr, int opacity = 255):
        m_type(Type::radial_gradient),
        m_opacity(rvg::color::int_to_uint8_t(opacity)) {
        new (&m_union.radial_gradient_ptr)
            RadialGradientPtr(radial_gradient_ptr);
    }

    Paint(const TexturePtr &texture_ptr, int opacity = 255):
        m_type(Type::texture),
        m_opacity(rvg::color::int_to_uint8_t(opacity)) {
        new (&m_union.texture_ptr) TexturePtr(texture_ptr);
    }

    Paint &operator=(Paint &&other) {
        // ??D we should optimize for case where other.m_type == m_type
        // in this case we could avoid destruction and construction
        // and invoke the move assignment directly in the m_union field
        this->~Paint();
        new (this) Paint(std::move(other));
        return *this;
    }

    Paint &operator=(const Paint &other) {
        // ??D we should optimize for case where other.m_type == m_type
        // in this case we could avoid destruction and construction
        // and invoke the copy assignment directly in the m_union field
        this->~Paint();
        new (this) Paint(std::move(other));
        return *this;
    }

    const Type &type(void) const {
        return m_type;
    }

    const uint8_t &opacity(void) const {
        return m_opacity;
    }

    SolidColor solid_color(void) const {
        return m_union.solid_color;
    }

    const LinearGradientPtr linear_gradient_ptr(void) const {
        return m_union.linear_gradient_ptr;
    }

    const LinearGradient &linear_gradient(void) const {
        return *m_union.linear_gradient_ptr;
    }

    const RadialGradientPtr radial_gradient_ptr(void) const {
        return m_union.radial_gradient_ptr;
    }

    const RadialGradient &radial_gradient(void) const {
        return *m_union.radial_gradient_ptr;
    }

    const TexturePtr texture_ptr(void) const {
        return m_union.texture_ptr;
    }

    const Texture &texture(void) const {
        return *m_union.texture_ptr;
    }

};

using PaintPtr = std::shared_ptr<Paint>;

} } // namespace rvg::paint

#endif
