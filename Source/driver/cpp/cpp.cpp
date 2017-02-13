#include <sstream>

#include <lua.hpp>
#include "description/lua.h"
#include "compat/compat.h"

#include "util/indent.h"
#include "xform/xform.h"
#include "path/svg/filter/command-printer.h"
#include "path/svg/filter/instruction-to-command.h"
#include "color/named.h"
#include "stroke/style.h"
#include "scene/iscene.h"
#include "image/iimage.h"
#include "image/pngio.h"
#include "image/image.h"

#include "driver/cpp/cpp.h"

using rvg::shape::Shape;
using rvg::color::RGBA8;
using rvg::shape::Circle;
using rvg::shape::Triangle;
using rvg::shape::Rect;
using rvg::shape::Polygon;
using rvg::path::Path;
using rvg::paint::Paint;
using rvg::paint::Spread;
using rvg::paint::Texture;
using rvg::paint::LinearGradient;
using rvg::paint::RadialGradient;
using rvg::paint::Ramp;
using rvg::stroke::Style;
using rvg::xform::Xform;

using rvg::math::is_almost_zero;
using rvg::math::is_almost_one;
using rvg::math::is_almost_equal;

void print_rotation(float c, float s, std::ostream &out) {
    if (!is_almost_one(c) || !is_almost_zero(s)) {
        out << ".rotated(" << rvg::math::deg(std::atan2(s, c)) << ')';
    }
}

void print_scaling(float sx, float sy, std::ostream &out) {
    if (is_almost_equal(sx, sy)) {
        float avg = 0.5f*(sx+sy);
        if (!is_almost_one(avg)) {
            out << ".scaled(" << avg << ')';
        }
    } else {
        out << ".scaled(" << sx << ',' << sy << ')';
    }
}

void print_translation(float tx, float ty, std::ostream &out) {
    if (!is_almost_zero(tx) || !is_almost_zero(ty)) {
        out << ".translated(" << tx << ',' << ty << ')';
    }
}

void print_linear(float a, float b, float c, float d, std::ostream &out) {
    if (is_almost_zero(a*b+c*d) && is_almost_one(a*d-b*c) &&
        is_almost_one(a*a+c*c) && is_almost_one(b*b+d*d)) {
        return print_rotation(a, c, out);
    } else if (is_almost_zero(b) && is_almost_zero(c)) {
        return print_scaling(a, d, out);
    } else {
        out << ".linear(" << a << ',' << b << ',' << c << ',' << d << ')';
    }
}

void print_affinity(float a, float b, float tx,
   float c, float d, float ty, std::ostream &out) {
    if (is_almost_zero(tx) && is_almost_zero(ty)) {
        return print_linear(a, b, c, d, out);
    } else {
        if (is_almost_one(a) && is_almost_zero(b) &&
            is_almost_zero(c) && is_almost_one(d)) {
            return print_translation(tx, ty, out);
        } else {
            out << ".affine(" << a << ',' << b << ',' << tx << ','
                  << c << ',' << d << ',' << ty << ')';
        }
    }
}

void print_projectivity(float a, float b, float c,
   float d, float e, float f,
   float g, float h, float i, std::ostream &out) {
    if (is_almost_zero(g) && is_almost_zero(h) && is_almost_one(i)) {
        return print_affinity(a, b, c, d, e, f, out);
    } else {
        out << ".projected(" << a << ',' << b << ',' << c << ','
              << d << ',' << e << ',' << f << ','
              << g << ',' << h << ',' << i << ')';
    }
}

void print_xform(const Xform &xf, std::ostream &out) {
    print_affinity(xf[0][0], xf[0][1], xf[0][2], xf[1][0], xf[1][1], xf[1][2], out);
}

void print_path(const Path &path, std::ostream &out) {
    out << "path({";
    path.iterate(rvg::path::svg::filter::make_instruction_to_command(
        rvg::path::svg::filter::make_command_printer(out, ',')));
    out << "})";
}

void print_circle(const Circle &circle, std::ostream &out) {
    out << "circle(" << circle.cx() << ',' << circle.cy()
        << ',' << circle.r()  << ')';
}

void print_triangle(const Triangle &triangle, std::ostream &out) {
    out << "triangle("
        << triangle.x1() << ',' << triangle.y1() << ','
        << triangle.x2() << ',' << triangle.y2() << ','
        << triangle.x3() << ',' << triangle.y3() << ')';
}

void print_rect(const Rect &rect, std::ostream &out) {
    out << "rect("
        << rect.x() << ',' << rect.y() << ','
        << rect.width() << ',' << rect.height() << ')';
}

void print_polygon(const Polygon &polygon, std::ostream &out) {
    out << "polygon({";
    const auto &coords = polygon.coordinates();
    if (!coords.empty()) {
        out << coords[0];
        for (unsigned i = 1; i < coords.size(); ++i) {
            out << ',' << coords[i];
        }
    }
    out << "})";
}

void print_style(const Style &st, std::ostream &out) {
	if (st.width() != Style::default_width) {
		out << ".stroked(" << st.width() << ')';
	}
    if (st.join() != Style::default_join ||
        st.miter_limit() != Style::default_miter_limit) {
        out << ".joined(join::" << st.join();
        if (st.miter_limit() != Style::default_miter_limit) {
             out << ',' << st.miter_limit();
        }
        out << ')';
    }
    if (st.cap() != Style::default_cap) {
        out << ".capped(cap::" << st.cap() << ")";
    }
    if (!st.dash_array().empty()) {
        out << ".dashed({";
        for (float d: st.dash_array()) out << d << ",";
        out << "}";
        if (st.phase_reset() != Style::default_phase_reset ||
            st.initial_phase() != Style::default_initial_phase) {
            out << ',' << st.initial_phase() << ',' << st.phase_reset();
        }
        out << ')';
    }
    if (st.method() != Style::default_method) {
        out << ".by(method::" << st.method() << ')';
    }
}

void print_shape(const Shape &shape, std::ostream &out) {
    using Type = Shape::Type;
    switch (shape.type()) {
        case Type::path:
            print_path(shape.path(), out);
            break;
        case Type::circle:
            print_circle(shape.circle(), out);
            break;
        case Type::triangle:
            print_triangle(shape.triangle(), out);
            break;
        case Type::rect:
            print_rect(shape.rect(), out);
            break;
        case Type::polygon:
            print_polygon(shape.polygon(), out);
            break;
        case Type::stroke:
            print_shape(shape.stroke().shape(), out);
            print_style(shape.stroke().style(), out);
            break;
        default:
            break;
    }
}

void print_rgba8(const RGBA8 &c, std::ostream &out) {
    using rvg::color::named::rgba8_to_string;
    const auto found = rgba8_to_string.find(c);
    if (found == rgba8_to_string.end()) {
        if (c.a() != 255) {
            out << "rgba8("
                << static_cast<int>(c.r()) << ','
                << static_cast<int>(c.g()) << ','
                << static_cast<int>(c.b()) << ','
                << static_cast<int>(c.a()) << ')';
        } else {
            out << "rgb8("
                << static_cast<int>(c.r()) << ','
                << static_cast<int>(c.g()) << ','
                << static_cast<int>(c.b()) << ')';
        }
    } else {
        out << "color::" << found->second;
    }
}

void print_solid_color(const RGBA8 &c, uint8_t opacity, std::ostream &out) {
    if (opacity != 255) out << "solid_color(";
    print_rgba8(c, out);
    if (opacity != 255) out << ',' << opacity << ')';
}

void print_ramp(const Ramp &r, std::ostream &out) {
    out << "ramp(spread::" << r.spread() << ",{";
    for (const auto &stop: r.stops()) {
        out << '{' << stop.offset() << ',';
        print_rgba8(stop.color(), out);
        out << "},";
    }
    out << "})";
}

void print_linear_gradient(const LinearGradient &lingrad,
    uint8_t opacity, std::ostream &out) {
    out << "linear_gradient(";
    print_ramp(lingrad.ramp(), out);
    out << ',' << lingrad.x1() << ',' << lingrad.y1() << ',' << lingrad.x2()
        << ',' << lingrad.y2();
    if (opacity < 255) out << ',' << static_cast<int>(opacity);
    out << ')';
}

void print_radial_gradient(const RadialGradient &radgrad,
    uint8_t opacity, std::ostream &out) {
    out << "radial_gradient(";
    print_ramp(radgrad.ramp(), out);
    out << ',' << radgrad.cx() << ',' << radgrad.cy() << ',' << radgrad.fx() << ',' << radgrad.fy() << ',' << radgrad.r();
    if (opacity < 255) out << ',' << static_cast<int>(opacity);
    out << ')';
}

void print_texture(const Texture &texture, uint8_t opacity, std::ostream &out) {
    out << "texture(";
    if (texture.spread() != Spread::pad) out << "spread::" <<
        texture.spread() << ",";
    out << "image::png::load(base64::decode(R\"-(\n";
    std::string s;
    if (texture.image().channel_type() == rvg::image::ChannelType::channel_uint8_t)
        rvg::image::pngio::store<uint8_t>(&s, texture.image_ptr());
    else
        rvg::image::pngio::store<uint16_t>(&s, texture.image_ptr());
    out << rvg::base64::encode(s);
    out << ")-\"))";
    if (opacity != 255)
        out << ',' << static_cast<int>(opacity);
    out << ')';
}

void print_paint(const Paint &paint, std::ostream &out) {
    using Type = Paint::Type;
    switch (paint.type()) {
        case Type::solid_color:
            print_solid_color(paint.solid_color(), paint.opacity(), out);
            break;
        case Type::linear_gradient:
            print_linear_gradient(paint.linear_gradient(), paint.opacity(), out);
            break;
        case Type::radial_gradient:
            print_radial_gradient(paint.radial_gradient(), paint.opacity(), out);
            break;
        case Type::texture:
            print_texture(paint.texture(), paint.opacity(), out);
        default:
            break;
    }
}

class ScenePrinter final: public rvg::scene::IScene<ScenePrinter> {
public:
    ScenePrinter(std::ostream &out): m_out(out), m_nl(2) { ; }

private:
    std::ostream &m_out;
    rvg::util::Indent m_nl;

    const char *winding_rule_prefix(WindingRule w) {
        switch (w) {
            case WindingRule::non_zero: return "";
            case WindingRule::zero: return "z";
            case WindingRule::odd: return "eo";
            case WindingRule::even: return "e";
            default: return "uknown";
        }
    }

    friend rvg::scene::IScene<ScenePrinter>;

    void do_painted_element(WindingRule winding_rule, const Shape &shape,
        const Paint &paint) {
        m_out << m_nl << winding_rule_prefix(winding_rule) << "fill(";
        print_shape(shape, m_out);
        print_xform(shape.xf(), m_out);
        m_out << ", ";
        print_paint(paint, m_out);
        m_out << "),";
    }

    void do_stencil_element(WindingRule winding_rule, const Shape &shape) {
        m_out << m_nl << winding_rule_prefix(winding_rule) << "stencil(";
        print_shape(shape, m_out);
        print_xform(shape.xf(), m_out);
        m_out << "),";
    }

    void do_begin_clip(uint16_t depth) {
        (void) depth;
        m_out << m_nl << "clip({";
        m_nl++;
    }

    void do_activate_clip(uint16_t depth) {
        (void) depth;
        m_nl--;
        m_out << m_nl << "},{";
        m_nl++;
    }

    void do_end_clip(uint16_t depth) {
        (void) depth;
        m_nl--;
        m_out << m_nl << "}),";
    }

    void do_begin_fade(uint16_t depth, uint8_t opacity) {
        (void) depth;
        m_out << m_nl << "fade(" << static_cast<int>(opacity)
            << ", {\n";
        m_nl++;
    }

    void do_end_fade(uint16_t depth, uint8_t opacity) {
        (void) depth; (void) opacity;
        m_nl--;
        m_out << m_nl << "}),";
    }

    void do_begin_blur(uint16_t depth, float radius) {
        (void) depth;
        m_out << m_nl << "blur(" << radius << ", {";
        m_nl++;
    }

    void do_end_blur(uint16_t depth, float radius) {
        (void) depth; (void) radius;
        m_nl--;
        m_out << m_nl << "}),";
    }

    void do_begin_transform(uint16_t depth, const Xform &xf) {
        (void) depth;
        m_out << m_nl << "transform(identity()";
        print_xform(xf, m_out);
        m_out << ", {";
        m_nl++;
    }

    void do_end_transform(uint16_t depth, const Xform &xf) {
        (void) depth; (void) xf;
        m_nl--;
        m_out << m_nl << "}),";
    }
};


namespace rvg {
    namespace driver {
        namespace cpp {

Accelerated accelerate(const XformableScene &xs, const Viewport &vp) {
    (void) vp;
    return xs;
}

void render(const Accelerated &accel, const Viewport &vp, std::ostream &out,
    const std::vector<std::string> &args) {
    (void) args;
    ScenePrinter sp(out);
    out << "// Automatically generated. Do not modify.";
    out << "\n#include \"description/description.h\"";
    out << "\nrvg::description::Description load(void) {";
    out << "\n  using namespace rvg::description;";
    out << "\n  using rvg::scene::WindingRule;";
    const auto &c = vp.corners();
    out << "\n  auto w = window(" << c[0] << ',' << c[1] << ',' << c[2]
        << ',' << c[3] << ");";
    out << "\n  auto v = viewport(" << c[0] << ',' << c[1] << ',' << c[2]
        << ',' << c[3] << ");";
    out << "\n  auto xs = scene({";
    accel.scene().iterate(sp);
    out << "\n  })";
    print_xform(accel.xf(), out);
    out << ";\n  return Description{xs, w, v};";
    out << "\n}\n";
}

} } } // namespace rvg::driver::cpp

// Lua version of the rvg::driver::cpp::accelerate function
// We know there is no acceleration. So we simply do nothing
// and return the scene itself (the first argument) unmodified.
static int luaaccelerate(lua_State *L) {
    lua_settop(L, 1);
    return 1;
}

// Lua version of the rvg::driver::cpp::render function
static int luarender(lua_State *L) {
    auto a = rvg::description::lua::checkxformablescene(L, 1);
    auto v = rvg::description::lua::checkviewport(L, 2);
    FILE *f = compat_check_file(L, 3);
    std::ostringstream sout;
    rvg::driver::cpp::render(a, v, sout);
    fwrite(sout.str().data(), 1, sout.str().size(), f);
    return 0;
}

// List of Lua functions exported into driver table
static const luaL_Reg modcpp[] = {
    {"render", luarender },
    {"accelerate", luaaccelerate },
    {NULL, NULL}
};

// Lua function invoked to be invoked by require"driver.cpp"
extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_driver_cpp_cpp(lua_State *L) {
    // build driver with all Lua functions needed to build
    // the scene description
    rvg::description::lua::newdriver(L);
	// add our accelerate and render functions to driver
    compat_setfuncs(L, modcpp, 1);
    return 1;
}
