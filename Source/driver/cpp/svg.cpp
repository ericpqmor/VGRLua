#include <unordered_map>
#include <string>
#include <sstream>

#include <lua.hpp>

#include "description/lua.h"
#include "compat/compat.h"

#include "util/indent.h"
#include "path/path.h"
#include "path/svg/filter/command-printer.h"
#include "path/svg/filter/instruction-to-command.h"
#include "path/filter/xformer.h"
#include "color/named.h"
#include "image/image.h"
#include "base64/base64.h"
#include "image/pngio.h"
#include "color/color.h"
#include "xform/xform.h"
#include "paint/paint.h"
#include "paint/spread.h"

#include "driver/cpp/svg.h"

namespace rvg {
    namespace driver {
        namespace svg {

using path::Path;
using xform::Xform;
using paint::Spread;
using scene::WindingRule;

Accelerated accelerate(const XformableScene &xs, const Viewport &vp) {
    (void) vp;
    return xs;
}

static const char *svg_winding_rule_name(WindingRule w) {
    switch (w) {
        case WindingRule::non_zero: return "nonzero";
        case WindingRule::zero: return "zero";
        case WindingRule::odd: return "evenodd";
        case WindingRule::even: return "even";
        default: return "";
    }
}

static void print_rotation(float c, float s, const char *title, std::ostream &out) {
    if (!math::is_almost_one(c) || !math::is_almost_zero(s)) {
        out << title << "=\"rotate(" << math::deg(std::atan2(s, c)) << ")\"";
    }
}

static void print_scaling(float sx, float sy, const char *title, std::ostream &out) {
    if (math::is_almost_equal(sx, sy)) {
        float avg = 0.5f*(sx+sy);
        if (!math::is_almost_one(avg)) {
            out << title << "=\"scale(" << avg << ")\"";
        }
    } else {
        out << title << "=\"scale(" << sx << ',' << sy << ")\"";
    }
}

static void print_translation(float tx, float ty, const char *title,
    std::ostream &out) {
    if (!math::is_almost_zero(tx) || !math::is_almost_zero(ty)) {
        out << title << "=\"translate(" << tx << ',' << ty << ")\"";
    }
}

static void print_linear(float a, float b, float c, float d, const char *title,
    std::ostream &out) {
    if (math::is_almost_zero(a*b+c*d) && math::is_almost_one(a*d-b*c) &&
        math::is_almost_one(a*a+c*c) && math::is_almost_one(b*b+d*d)) {
        return print_rotation(a, c, title, out);
    } else if (math::is_almost_zero(b) && math::is_almost_zero(c)) {
        return print_scaling(a, d, title, out);
    } else {
        out << title << "=\"matrix(" <<
            a << ',' << c << ',' <<
            b << ',' << d << ",0,0)\"";
    }
}

static void print_affinity(float a, float b, float tx,
   float c, float d, float ty, const char *title, std::ostream &out) {
    if (math::is_almost_zero(tx) && math::is_almost_zero(ty)) {
        return print_linear(a, b, c, d, title, out);
    } else {
        if (math::is_almost_one(a) && math::is_almost_zero(b) &&
        math::is_almost_zero(c) && math::is_almost_one(d)) {
            return print_translation(tx, ty, title, out);
        } else {
            out << title << "=\"matrix(" <<
                a << ',' << c << ',' <<
                b << ',' << d << ',' <<
                tx << ',' << ty << ")\"";
        }
    }
}

static void print_xform(const xform::Xform &xf, const char *title,
    std::ostream &out) {
    print_affinity(xf[0][0], xf[0][1], xf[0][2], xf[1][0], xf[1][1],
        xf[1][2], title, out);
}

static void print_path_data(const Path &path, const Xform &pre_xf,
    std::ostream &out) {
    out << " d=\"";
    if (!pre_xf.is_identity()) {
        path.iterate(
            path::filter::make_xformer(pre_xf,
                path::svg::filter::make_instruction_to_command(
                    path::svg::filter::make_command_printer(out, ' '))));
    } else {
        path.iterate(
            path::svg::filter::make_instruction_to_command(
                path::svg::filter::make_command_printer(out, ' ')));
    }
    out << "\"";
}

static void print_path_data(const Path &path, std::ostream &out) {
    out << " d=\"";
    path.iterate(
        path::svg::filter::make_instruction_to_command(
            path::svg::filter::make_command_printer(out, ' ')));
    out << "\"";
}


class SVGPaintedPrinter final: public scene::IScene<SVGPaintedPrinter> {
    util::Indent &m_nl;
    const std::unordered_map<std::string, int> &m_map;
    const xform::Xform &m_screen_xf;
	std::ostream &m_out;
	int m_shape_id, m_clippath_id;
    std::vector<int> m_active_clips, m_not_yet_active_clips;
public:
	SVGPaintedPrinter(
        util::Indent &nl,
        const std::unordered_map<std::string, int> &map,
        const xform::Xform &screen_xf,
        std::ostream &out):
        m_nl(nl),
        m_map(map),
        m_screen_xf(screen_xf),
        m_out(out),
        m_shape_id(0),
        m_clippath_id(0)
        { ; }

private:
    using Style = stroke::Style;

    void print_paint(const char *mode, const Shape &shape, const Paint &paint) {
        (void) shape;
        m_out << ' ' << mode << "=\"";
        switch (paint.type()) {
            case Paint::Type::solid_color: {
				const auto c = paint.solid_color();
                const auto found = color::named::rgba8_to_string.find(c);
				if (found == color::named::rgba8_to_string.end()) {
                    m_out << "rgb("
                        << static_cast<int>(c.r()) << ','
                        << static_cast<int>(c.g()) << ','
                        << static_cast<int>(c.b()) << ')';
				} else {
					m_out << found->second;
				}
                if (c.a() != 255 || paint.opacity() != 255) {
                    m_out << "\" " << mode << "-opacity=\"" <<
                        color::uint8_t_to_unorm(c.a()) *
                        color::uint8_t_to_unorm(paint.opacity());
                }
                m_out << '"';
                break;
            }
            case Paint::Type::linear_gradient:
            case Paint::Type::radial_gradient: {
                std::ostringstream s;
                s << &paint;
                const auto found = m_map.find(s.str());
                if (found != m_map.end()) {
                    m_out << "url(#gradient" << found->second << ")";
                    if (paint.opacity() != 255) {
                        m_out << "\" " << mode << "-opacity=\"" <<
                            color::uint8_t_to_unorm(paint.opacity());
                    }
                }
                m_out << '"';
                break;
            }
            case Paint::Type::texture: {
                std::ostringstream s;
                s << &paint;
                const auto found = m_map.find(s.str());
                if (found != m_map.end()) {
                    m_out << "url(#texture" << found->second << ")";
                    if (paint.opacity() != 255) {
                        m_out << "\" " << mode << "-opacity=\"" <<
                            color::uint8_t_to_unorm(paint.opacity());
                    }
                }
                m_out << '"';
                break;
            }
            default:
                break;
        }
    }

    void print_stroke_style(const Style &st) {
        m_out << " stroke-width=\"" << st.width() << '"';
        if (st.join() != Style::default_join) {
            m_out << " stroke-linejoin=\"" << st.join() << '"';
        }
        if (st.miter_limit() != Style::default_miter_limit) {
            m_out << " stroke-miterlimit=\"" << st.miter_limit() << '"';
        }
        if (st.cap() != Style::default_cap) {
            m_out << " stroke-linecap=\"" << st.cap() << '"';
        }
        if (!st.dash_array().empty()) {
            m_out << " stroke-dasharray=\"";
            for (float d: st.dash_array())
                m_out << d << ' ';
            m_out << '"';
        }
        if (st.phase_reset() != Style::default_phase_reset) {
            m_out << " stroke-phasereset=\"" << st.phase_reset() << '"';
        }
        if (st.initial_phase() != Style::default_initial_phase) {
            m_out << " stroke-dashoffset=\"" << st.initial_phase() << '"';
        }
    }


    friend scene::IScene<SVGPaintedPrinter>;

    void do_painted_element(WindingRule wr, const Shape &shape,
        const Paint &paint) {
        const char *mode = nullptr;
        const stroke::Style *stroke_style = nullptr;
        Shape path_shape;
        xform::Xform pre_xf = xform::Identity();
        xform::Xform post_xf = shape.xf();
        if (shape.type() == Shape::Type::stroke &&
            shape.stroke().style().method() == stroke::Method::driver) {
            mode = "stroke";
            stroke_style = &shape.stroke().style();
            pre_xf = shape.stroke().shape().xf();
            // convert shape to be stroked into a path
            path_shape = shape.stroke().shape().as_path_shape(
                shape.xf().transformed(m_screen_xf));
        } else {
            mode = "fill";
            path_shape = shape.as_path_shape(m_screen_xf);
        }
        m_out << m_nl << "<path id=\"shape" << m_shape_id << '"';
        m_out << " fill-rule=\"" << svg_winding_rule_name(wr) << '"';
        if (stroke_style) {
            m_out << " fill=\"none\"";
            print_stroke_style(*stroke_style);
        }
        print_xform(post_xf, " transform", m_out);
        print_paint(mode, shape, paint);
        print_path_data(path_shape.path(), pre_xf, m_out);
        m_out << "/>";
        ++m_shape_id;
    }

    void do_stencil_element(WindingRule winding_number, const Shape &shape) {
        (void) winding_number; (void) shape;
        // all stencil elements are in inside the <defs> section
    }

    void do_begin_clip(uint16_t depth) {
        (void) depth;
        m_not_yet_active_clips.push_back(m_clippath_id);
        ++m_clippath_id;
    }

    void do_activate_clip(uint16_t depth) {
        (void) depth;
        m_active_clips.push_back(m_not_yet_active_clips.back());
        m_not_yet_active_clips.pop_back();
        if (m_not_yet_active_clips.empty()) {
            m_out << m_nl++ << "<g clip-path=\"url(#clip" <<
                m_active_clips.back() << ")\">";
        }
    }

    void do_end_clip(uint16_t depth) {
        (void) depth;
        m_active_clips.pop_back();
        if (m_not_yet_active_clips.empty()) {
            m_out << --m_nl << "</g>";
        }
    }

    void do_begin_fade(uint16_t depth, uint8_t opacity) {
        (void) depth;
        if (opacity != 255) {
            m_out << m_nl++ << "<g opacity=\"" <<
                color::uint8_t_to_unorm(opacity) << "\">";
        }
    }

    void do_end_fade(uint16_t depth, uint8_t opacity) {
        (void) depth;
        if (opacity != 255) {
            m_out << --m_nl << "</g>";
        }
    }

    void do_begin_blur(uint16_t depth, float radius) {
        (void) depth;
        if (!math::is_almost_zero(radius)) {
            std::ostringstream s;
            s << "blur " << radius;
            const auto found = m_map.find(s.str());
            m_out << m_nl++ << "<g filter=\"url(#blur" <<
                ((found != m_map.end())? found->second: -1) << ")\">";
        }
    }

    void do_end_blur(uint16_t depth, float radius) {
        (void) depth;
        if (!math::is_almost_zero(radius)) {
            m_out << --m_nl << "</g>";
        }
    }

    void do_begin_transform(uint16_t depth, const xform::Xform &xf) {
        (void) depth;
        if (m_not_yet_active_clips.empty()) {
            m_out << m_nl++ << "<g";
            print_xform(xf, " transform", m_out);
            m_out << '>';
        }
    }

    void do_end_transform(uint16_t depth, const xform::Xform &xf) {
        (void) depth; (void) xf;
        if (m_not_yet_active_clips.empty()) {
            m_out << --m_nl << "</g>";
        }
    }
};

class SVGPaintStencilPrinter final:
    public scene::IScene<SVGPaintStencilPrinter> {
    util::Indent &m_nl;
    std::unordered_map<std::string, int> &m_map;
    const xform::Xform &m_screen_xf;
	std::ostream &m_out;
	int m_blur_id, m_gradient_id, m_texture_id, m_stencil_id, m_clippath_id;
    Xform m_stencil_xf;
    std::vector<Xform> m_stencil_xf_stack;
    std::vector<int> m_active_clips, m_not_yet_active_clips;
public:
	SVGPaintStencilPrinter(
        util::Indent &nl,
        std::unordered_map<std::string, int> &map,
        const xform::Xform &screen_xf,
        std::ostream &out):
        m_nl(nl),
        m_map(map),
        m_screen_xf(screen_xf),
        m_out(out),
        m_blur_id(0),
        m_gradient_id(0),
        m_texture_id(0),
        m_stencil_id(0),
        m_clippath_id(0)
        { ; }

private:
    using Style = stroke::Style;

    const char *svg_spread_name(Spread s) {
        switch (s) {
            case Spread::pad: return "repeat";
            case Spread::repeat: return "pad";
            case Spread::reflect: return "reflect";
            case Spread::transparent: return "transparent";
            default: return "";
        }
    }

    void print_ramp(const paint::Ramp &ramp) {
        using color::named::rgba8_to_string;
        for (const auto &stop: ramp.stops()) {
            m_out << m_nl << "<stop offset=\"" << stop.offset() <<
                "\" stop-color=\"";
            const auto found = rgba8_to_string.find(stop.color());
            if (found == rgba8_to_string.end()) {
                m_out << "rgb("
                    << static_cast<int>(stop.color().r()) << ','
                    << static_cast<int>(stop.color().g()) << ','
                    << static_cast<int>(stop.color().b()) << ')';
            } else {
                m_out << found->second;
            }
            if (stop.color().a() < 255) {
                m_out << "\" stop-opacity=\"" <<
                    color::uint8_t_to_unorm(stop.color().a());
            }
            m_out << "\"/>";
        }
    }

    void print_linear_gradient(const Shape &shape, const Paint &paint) {
        (void) shape;
        std::ostringstream s;
        s << &paint;
        auto found = m_map.insert({s.str(), m_gradient_id});
        if (found.second) {
            const auto &linear_gradient = paint.linear_gradient();
            m_out << m_nl << "<linearGradient id=\"gradient" << m_gradient_id <<
                "\" gradientUnits=\"userSpaceOnUse\" x1=\"" <<
                linear_gradient.x1() << "\" y1=\"" <<
                linear_gradient.y1() << "\" x2=\"" <<
                linear_gradient.x2() << "\" y2=\"" <<
                linear_gradient.y2() << "\" spreadMethod=\"" <<
                svg_spread_name(linear_gradient.ramp().spread()) << "\"";
            print_xform(paint.xf().transformed(shape.xf().inverse()),
                " gradientTransform", m_out);
            m_out << ">";
            m_nl++;
            print_ramp(linear_gradient.ramp());
            m_nl--;
            m_out << m_nl << "</linearGradient>";
            ++m_gradient_id;
        }
    }

    void print_radial_gradient(const Shape &shape, const Paint &paint) {
        (void) shape;
        std::ostringstream s;
        s << &paint;
        auto found = m_map.insert({s.str(), m_gradient_id});
        if (found.second) {
            const auto &radial_gradient = paint.radial_gradient();
            m_out << m_nl << "<radialGradient id=\"gradient" << m_gradient_id <<
                "\" gradientUnits=\"userSpaceOnUse\" cx=\"" <<
                radial_gradient.cx() << "\" cy=\"" <<
                radial_gradient.cy() << "\" fx=\"" <<
                radial_gradient.fx() << "\" fy=\"" <<
                radial_gradient.fy() << "\" r=\"" <<
                radial_gradient.r() << "\" spreadMethod=\"" <<
                svg_spread_name(radial_gradient.ramp().spread()) << "\"";
            print_xform(paint.xf().transformed(shape.xf().inverse()),
                " gradientTransform", m_out);
            m_out << ">";
            m_nl++;
            print_ramp(radial_gradient.ramp());
            m_nl--;
            m_out << m_nl << "</radialGradient>";
            ++m_gradient_id;
        }
    }

    void print_texture(const Shape &shape, const Paint &paint) {
        (void) shape;
        std::ostringstream s;
        s << &paint;
        auto found = m_map.insert({s.str(), m_texture_id});
        if (found.second) {
            const auto &texture = paint.texture();
            m_out << m_nl++ << "<pattern id=\"texture" << m_texture_id <<
                "\" patternUnits=\"userSpaceOnUse\" width=\"1\" height=\"1\" preserveAspectRatio=\"none\" ";
            print_xform(paint.xf().transformed(shape.xf().inverse()),
                " patternTransform", m_out);
            m_out << '>';
            m_out << m_nl << "<image id=\"image" << m_texture_id <<
                "\" width=\"1\" height=\"1\" preserveAspectRatio=\"none\"" <<
                " transform=\"scale(1,-1) translate(0,-1)\" xlink:href=\"" <<
                " data:image/png;base64,\n";
            std::string simg;
            if (texture.image().channel_type() == image::ChannelType::channel_uint8_t)
                rvg::image::pngio::store<uint8_t>(&simg, texture.image_ptr());
            else
                rvg::image::pngio::store<uint16_t>(&simg, texture.image_ptr());
            m_out << rvg::base64::encode(simg) << "\"/>";
            m_out << --m_nl << "</pattern>";
            ++m_texture_id;
        }
    }

    friend scene::IScene<SVGPaintStencilPrinter>;

    void do_painted_element(WindingRule, const Shape &shape,
        const Paint &paint) {
        switch (paint.type()) {
            case Paint::Type::solid_color:
                break;
            case Paint::Type::linear_gradient:
                print_linear_gradient(shape, paint);
                break;
            case Paint::Type::radial_gradient:
                print_radial_gradient(shape, paint);
                break;
            case Paint::Type::texture:
                print_texture(shape, paint);
                break;
        }
    }

    void do_stencil_element(WindingRule winding_rule, const Shape &shape) {
        m_out << m_nl << "<path id=\"stencil" << m_stencil_id << '"';
        m_out << " clip-rule=\"" << svg_winding_rule_name(winding_rule) << '"';
        Shape path_shape = shape.as_path_shape(m_screen_xf);
        print_path_data(path_shape.path(), m_out);
        print_xform(shape.xf().transformed(m_stencil_xf), " transform", m_out);
        m_out << "/>";
        ++m_stencil_id;
    }

    void do_begin_clip(uint16_t depth) {
        (void) depth;
        m_not_yet_active_clips.push_back(m_clippath_id);
        ++m_clippath_id;
    }

    void do_activate_clip(uint16_t depth) {
        (void) depth;
        m_active_clips.push_back(m_not_yet_active_clips.back());
        m_not_yet_active_clips.pop_back();
    }

    void do_end_clip(uint16_t depth) {
        (void) depth;
        m_active_clips.pop_back();
    }

    void do_begin_fade(uint16_t depth, uint8_t opacity) {
        (void) depth; (void) opacity;
    }

    void do_end_fade(uint16_t depth, uint8_t opacity) {
        (void) depth; (void) opacity;
    }

    void do_begin_blur(uint16_t depth, float radius) {
        (void) depth;
        if (!math::is_almost_zero(radius)) {
            std::ostringstream s;
            s << "blur " << radius;
            auto found = m_map.insert({s.str(), m_blur_id});
            if (found.second) {
                m_out << m_nl++ << "<filter id=\"blur" << m_blur_id << "\">";
                m_out << m_nl << "<feGaussianBlur stdDeviation=\"" << radius
                    << "\"/>";
                m_out << --m_nl << "</filter>";
                ++m_blur_id;
            }
        }
    }

    void do_end_blur(uint16_t depth, float radius) {
        (void) depth;
        if (!math::is_almost_zero(radius)) {
        }
    }

    void do_begin_transform(uint16_t depth, const xform::Xform &xf) {
        (void) depth;
        // if the transformation happened within a stencil
        // definition, we accumulate it
        if (!m_not_yet_active_clips.empty()) {
            m_stencil_xf_stack.push_back(m_stencil_xf);
            m_stencil_xf = xf.transformed(m_stencil_xf);
        }
    }

    void do_end_transform(uint16_t depth, const xform::Xform &xf) {
        (void) depth; (void) xf;
        if (!m_not_yet_active_clips.empty()) {
            m_stencil_xf = m_stencil_xf_stack.back();
            m_stencil_xf_stack.pop_back();
        }
    }
};

class SVGClipPrinter final: public scene::IScene<SVGClipPrinter> {
    util::Indent &m_nl;
    const std::unordered_map<std::string, int> &m_map;
    const xform::Xform &m_screen_xf;
	std::ostream &m_out;
	int m_stencil_id, m_clippath_id;
    std::vector<int> m_active_clips, m_not_yet_active_clips, m_nested_clips;
public:
	SVGClipPrinter(
        util::Indent &nl,
        const std::unordered_map<std::string, int> &map,
        const xform::Xform &screen_xf,
        std::ostream &out):
        m_nl(nl),
        m_map(map),
        m_screen_xf(screen_xf),
        m_out(out),
        m_stencil_id(0),
        m_clippath_id(0)
        { ; }

private:
    using Style = stroke::Style;

    friend scene::IScene<SVGClipPrinter>;

    void do_painted_element(WindingRule winding_rule, const Shape &shape,
        const Paint &paint) {
        (void) winding_rule;
        (void) shape;
        (void) paint;
    }

    void do_stencil_element(WindingRule winding_rule, const Shape &shape) {
        (void) winding_rule; (void) shape;
        m_out << m_nl << "<use xlink:href=\"#stencil" << m_stencil_id << '"';
        if (!m_nested_clips.empty()) {
            m_out << " clip-path=\"url(#clip" << m_nested_clips.back() << ")\"";
        }
        m_out << "/>";
        ++m_stencil_id;
    }

    void do_begin_clip(uint16_t depth) {
        (void) depth;
        m_out << m_nl++ << "<clipPath id=\"clip" << m_clippath_id << "\">";
        m_not_yet_active_clips.push_back(m_clippath_id);
        ++m_clippath_id;
    }

    void do_activate_clip(uint16_t depth) {
        (void) depth;
        m_out << --m_nl << "</clipPath>";
        m_active_clips.push_back(m_not_yet_active_clips.back());
        m_not_yet_active_clips.pop_back();
        if (!m_not_yet_active_clips.empty()) {
            m_nested_clips.push_back(m_active_clips.back());
        }
    }

    void do_end_clip(uint16_t depth) {
        (void) depth;
        if (!m_nested_clips.empty() &&
            m_nested_clips.back() == m_active_clips.back()) {
            m_nested_clips.pop_back();
        }
        m_active_clips.pop_back();
    }

    void do_begin_fade(uint16_t depth, uint8_t opacity) {
        (void) depth; (void) opacity;
    }

    void do_end_fade(uint16_t depth, uint8_t opacity) {
        (void) depth; (void) opacity;
    }

    void do_begin_blur(uint16_t depth, float radius) {
        (void) depth; (void) radius;
    }

    void do_end_blur(uint16_t depth, float radius) {
        (void) depth; (void) radius;
    }

    void do_begin_transform(uint16_t depth, const xform::Xform &xf) {
        (void) depth; (void) xf;
    }

    void do_end_transform(uint16_t depth, const xform::Xform &xf) {
        (void) depth; (void) xf;
    }
};

void render(const Accelerated &accel, const Viewport &vp, std::ostream &out,
    const std::vector<std::string> &args) {
    (void) args;
    int xl, yb, xr, yt;
    std::tie(xl, yb) = vp.bl();
    std::tie(xr, yt) = vp.tr();
    std::unordered_map<std::string, int> map;
    util::Indent nl;
    out << "<?xml version=\"1.0\" standalone=\"no\"?>\n" <<
       "<svg\n" <<
       "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n" <<
       "  xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n" <<
       "  xmlns:cc=\"http://creativecommons.org/ns#\"\n" <<
       "  xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n" <<
       "  xmlns:svg=\"http://www.w3.org/2000/svg\"\n" <<
       "  xmlns=\"http://www.w3.org/2000/svg\"\n" <<
       "  version=\"1.1\"\n" <<
       "  width=\"" << std::abs(xr-xl) <<
         "\" height=\"" << std::abs(yt-yb) << "\"\n" <<
           "  viewBox=\"" << std::min(xl,xr) << ' ' << std::min(yt,yb) << ' ' <<
            std::abs(xl-xr) << ' ' << std::abs(yt-yb) << "\"" << ">";
    Xform flip = xform::translation(0.f,-static_cast<float>(yb)).
        scaled(1.f,-1.f).translated(0.f,static_cast<float>(yt));
    Xform screen_xf = accel.xf().transformed(flip);
    ++nl;
    out << nl++ << "<defs>";
    // write stencil shape, gradient paints, and textures
	SVGPaintStencilPrinter psp(nl, map, screen_xf, out);
	accel.scene().iterate(psp);
    // write clip-paths
	SVGClipPrinter cp(nl, map, screen_xf, out);
	accel.scene().iterate(cp);
    out << --nl << "</defs>";
    out << nl++ << "<g";
    print_xform(flip, " transform", out);
    out << "> <!-- invert y -->";
    out << nl++ << "<g";
    print_xform(accel.xf(), " transform", out);
    out << "> <!-- window-viewport -->";
    // write painted shapes
	SVGPaintedPrinter pp(nl, map, screen_xf, out);
	accel.scene().iterate(pp);
    out << --nl << "</g>";
    out << --nl << "</g>";
    out << "\n</svg>\n";
}

} } } // namespace rvg::driver::svg

// Lua version of the rvg::driver::svg::accelerate function
// We know there is no acceleration. So we simply do nothing
// and return the scene itself (the first argument) unmodified.
static int luaaccelerate(lua_State *L) {
    lua_settop(L, 1);
    return 1;
}

// Lua version of the rvg::driver::svg::render function
static int luarender(lua_State *L) {
    auto accel = rvg::description::lua::checkxformablescene(L, 1);
    auto vp = rvg::description::lua::checkviewport(L, 2);
    FILE *f = compat_check_file(L, 3);
    std::ostringstream sout;
    rvg::driver::svg::render(accel, vp, sout);
    fwrite(sout.str().data(), 1, sout.str().size(), f);
    return 0;
}

// List of Lua functions exported into driver table
static const luaL_Reg modsvg[] = {
    {"render", luarender },
    {"accelerate", luaaccelerate },
    {NULL, NULL}
};

// Lua function invoked to be invoked by require"driver.svg"
extern "C"
#ifndef _WIN32
__attribute__((visibility("default")))
#else
__declspec(dllexport)
#endif
int luaopen_driver_cpp_svg(lua_State *L) {
	// build driver with all Lua functions needed to build
    // the scene description
	rvg::description::lua::newdriver(L); // driver mettab
    // add our accelerate and render functions to driver
    compat_setfuncs(L, modsvg, 1); // driver
    return 1;
}
