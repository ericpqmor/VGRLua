#include <type_traits>
#include <iterator>

#include "meta/meta.h"
#include "path/svg/parse.h"

#include "description/description.h"

namespace rvg {
    namespace description {

using rvg::color::unorm_to_uint8_t;
using rvg::scene::Scene;

struct Depths { int blur, fade, clip, xform; };

static void build_stencil(const std::vector<Stencil> &stencil, Depths &depths,
    Scene &forward);

static void build_painted(const std::vector<Painted> &painted, Depths &depths,
    Scene &forward);

Shape path(const char *svg) {
    auto p = std::make_shared<path::Path>();
    if (!path::svg::tokens_to_instructions(svg, *p))
        p->clear();
    return Shape(p);
}

Shape circle(float x, float y, float r) {
    return Shape(std::make_shared<shape::Circle>(x, y, r));
}

Shape triangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    return Shape(std::make_shared<shape::Triangle>(x1, y1, x2, y2, x3, y3));
}

Shape rect(float x, float y, float width, float height) {
    return Shape(std::make_shared<shape::Rect>(x, y, width, height));
}

Shape polygon(const std::initializer_list<float> &coordinates) {
    return Shape(std::make_shared<shape::Polygon>(coordinates));
}

Shape path(const ILToken &svg) {
    auto p = std::make_shared<path::Path>();
    if (!path::svg::tokens_to_instructions(svg, *p))
        p->clear();
    return Shape(p);
}

Painted nzfill(const Shape &shape, const Paint &paint) {
    return make_painted_primitive(scene::WindingRule::non_zero, shape, paint);
}

Painted zfill(const Shape &shape, const Paint &paint) {
    return make_painted_primitive(scene::WindingRule::zero, shape, paint);
}

Painted ofill(const Shape &shape, const Paint &paint) {
    return make_painted_primitive(scene::WindingRule::odd, shape, paint);
}

Painted efill(const Shape &shape, const Paint &paint) {
    return make_painted_primitive(scene::WindingRule::even, shape, paint);
}

Painted fill(const Shape &shape, const Paint &paint) {
    return make_painted_primitive(scene::WindingRule::non_zero, shape, paint);
}

Painted eofill(const Shape &shape, const Paint &paint) {
    return make_painted_primitive(scene::WindingRule::odd, shape, paint);
}

Painted fade(uint8_t opacity, const VPainted &painted) {
    return make_painted_faded(opacity, painted);
}

Painted fade(uint8_t opacity, const ILPainted &painted) {
    return make_painted_faded(opacity, painted);
}

Painted transform(const Xform &xf, const VPainted &painted) {
    return make_painted_xformed(xf, painted);
}

Painted transform(const Xform &xf, const ILPainted &painted) {
    return make_painted_xformed(xf, painted);
}

Painted blur(float radius, const VPainted &painted) {
    return make_painted_blurred(radius, painted);
}

Painted blur(float radius, const ILPainted &painted) {
    return make_painted_blurred(radius, painted);
}

Painted blur(float radius, const Painted &painted) {
    return blur(radius, { painted });
}

Stencil nzstencil(const Shape &shape) {
    return make_stencil_primitive(scene::WindingRule::non_zero, shape);
}

Stencil zstencil(const Shape &shape) {
    return make_stencil_primitive(scene::WindingRule::zero, shape);
}

Stencil ostencil(const Shape &shape) {
    return make_stencil_primitive(scene::WindingRule::odd, shape);
}

Stencil estencil(const Shape &shape) {
    return make_stencil_primitive(scene::WindingRule::even, shape);
}

Stencil stencil(const Shape &shape) {
    return make_stencil_primitive(scene::WindingRule::non_zero, shape);
}

Stencil eostencil(const Shape &shape) {
    return make_stencil_primitive(scene::WindingRule::odd, shape);
}

Painted clip(const VStencil &clipper, const VPainted &clippee) {
    return make_painted_clipped(clipper, clippee);
}

Painted clip(const ILStencil &clipper, const ILPainted &clippee) {
    return make_painted_clipped(clipper, clippee);
}

Painted clip(const Stencil &clipper, const ILPainted &clippee) {
    return clip({ clipper }, clippee);
}

Painted clip(const ILStencil &clipper, const Painted &clippee) {
    return clip(clipper, { clippee });
}

Painted clip(const Stencil &clipper, const Painted &clippee) {
    return clip({ clipper }, { clippee });
}

Stencil clip(const ILStencil &clipper, const ILStencil &clippee) {
    return make_stencil_clipped(clipper, clippee);
}

Stencil clip(const VStencil &clipper, const VStencil &clippee) {
    return make_stencil_clipped(clipper, clippee);
}

Stencil clip(const Stencil &clipper, const ILStencil &clippee) {
    return clip({ clipper }, clippee);
}

Stencil clip(const ILStencil &clipper, const Stencil &clippee) {
    return clip(clipper, { clippee });
}

Stencil clip(const Stencil &clipper, const Stencil &clippee) {
    return clip({ clipper }, { clippee });
}

Stencil transform(const Xform &xf, const ILStencil &stencil) {
    return make_stencil_xformed(xf, stencil);
}

Stencil transform(const Xform &xf, const VStencil &stencil) {
    return make_stencil_xformed(xf, stencil);
}

template <typename IT>
typename std::enable_if<std::is_same<
    typename std::iterator_traits<IT>::value_type, Stencil>::value>::type
build_stencil(IT first, IT last, Depths &depths, Scene &forward) {
    using ST = Stencil::Type;
    for ( ; first != last; ++first) {
        const auto &st = *first;
        if (st.type() == ST::primitive) {
            const auto &prim = st.primitive();
            forward.stencil_element(
                prim.winding_rule(),
                prim.shape().transformed(st.xf()));
        } else {
            // all stencil objects have their own xf
            // if the xf is not the identity, we bracket
            // around it with a transform bracket
            bool xformed = !st.xf().is_identity();
            if (xformed) {
                forward.begin_transform(depths.xform, st.xf());
                ++depths.xform;
            }
            switch (st.type()) {
                case ST::xformed:
                    // already bracketed outside
                    build_stencil(st.xformed().stencil(), depths, forward);
                    break;
                case ST::clipped: {
                    const auto &clipped = st.clipped();
                    forward.begin_clip(static_cast<uint16_t>(depths.clip++));
                    build_stencil(clipped.clipper(), depths, forward);
                    forward.activate_clip(static_cast<uint16_t>(depths.clip-1));
                    build_stencil(clipped.clippee(), depths, forward);
                    forward.end_clip(static_cast<uint16_t>(--depths.clip));
                    break;
                }
                default:
                    // do nothing
                    break;
            }
            if (xformed) {
                --depths.xform;
                forward.end_transform(depths.xform, st.xf());
            }
        }
    }
}

static void build_stencil(const std::vector<Stencil> &stencil, Depths &depths,
    Scene &forward) {
    build_stencil(stencil.begin(), stencil.end(), depths, forward);
}

template <typename IT>
typename std::enable_if<std::is_same<
    typename std::iterator_traits<IT>::value_type, Painted>::value>::type
build_painted(IT first, IT last, Depths &depths, Scene &forward) {
    using PT = Painted::Type;
    for ( ; first != last; ++first) {
        const Painted &p = *first;
        if (p.type() == PT::primitive) {
            const auto &prim = p.primitive();
            forward.painted_element(prim.winding_rule(),
                prim.shape().transformed(p.xf()),
                prim.paint().transformed(p.xf()));
        } else {
            // all painted objects have their own xf
            // if the xf is not the identity, we bracket
            // around it with a transform bracket
            bool xformed = !p.xf().is_identity();
            if (xformed) {
                forward.begin_transform(depths.xform, p.xf());
                ++depths.xform;
            }
            switch (p.type()) {
                case PT::faded: {
                    const auto &faded = p.faded();
                    forward.begin_fade(static_cast<uint16_t>(depths.fade++),
                        faded.opacity());
                    build_painted(faded.painted(), depths, forward);
                    forward.end_fade(static_cast<uint16_t>(--depths.fade),
                        faded.opacity());
                    break;
                }
                case PT::xformed: {
                    // already bracketed outside
                    build_painted(p.xformed().painted(), depths, forward);
                    break;
                }
                case PT::blurred: {
                    const auto &blurred = p.blurred();
                    forward.begin_blur(static_cast<uint16_t>(depths.blur++),
                        blurred.radius());
                    build_painted(blurred.painted(), depths, forward);
                    forward.end_blur(static_cast<uint16_t>(--depths.blur),
                        blurred.radius());
                    break;
                }
                case PT::clipped: {
                    const auto &clipped = p.clipped();
                    forward.begin_clip(static_cast<uint16_t>(depths.clip++));
                    build_stencil(clipped.clipper(), depths, forward);
                    forward.activate_clip(static_cast<uint16_t>(depths.clip-1));
                    build_painted(clipped.clippee(), depths, forward);
                    forward.end_clip(static_cast<uint16_t>(--depths.clip));
                    break;
                }
                default:
                    // do nothing
                    break;
            }
            if (xformed) {
                --depths.xform;
                forward.end_transform(depths.xform, p.xf());
            }
        }
    }
}

static void build_painted(const std::vector<Painted> &painted, Depths &depths,
    Scene &forward) {
    build_painted(painted.begin(), painted.end(), depths, forward);
}

template <typename IT>
typename std::enable_if<std::is_same<
    typename std::iterator_traits<IT>::value_type, Painted>::value,
XformableScene>::type scene(IT first, IT last) {
    scene::ScenePtr s = std::make_shared<scene::Scene>();
    Depths depths{0,0,0,0};
    build_painted(first, last, depths, *s);
    return XformableScene(s);
}

XformableScene scene(const ILPainted &painted) {
    return scene(painted.begin(), painted.end());
}

XformableScene scene(const std::vector<Painted> &painted) {
    return scene(painted.begin(), painted.end());
}

XformableScene scene(const Painted &painted) {
    return scene({ painted });
}

Paint solid_color(const rvg::color::RGBA8 &color, uint8_t opacity) {
    return Paint(color, opacity);
}

RampPtr ramp(Spread spread, const std::initializer_list<Stop> &stops) {
    return std::make_shared<paint::Ramp>(spread, stops);
}

RampPtr ramp(const std::initializer_list<Stop> &stops) {
    return std::make_shared<paint::Ramp>(Spread::pad, stops);
}

Paint texture(Spread spread, const IImagePtr &image_ptr, uint8_t opacity) {
    return Paint(std::make_shared<paint::Texture>(spread, image_ptr), opacity);
}

Paint texture(const IImagePtr &image_ptr, uint8_t opacity) {
    return Paint(std::make_shared<paint::Texture>(Spread::pad, image_ptr),
        opacity);
}

Paint linear_gradient(const RampPtr &ramp_ptr, float x1, float y1, float x2,
    float y2, uint8_t opacity) {
    return Paint(std::make_shared<paint::LinearGradient>(ramp_ptr,
        x1, y1, x2, y2), opacity);
}

Paint radial_gradient(const RampPtr &ramp_ptr, float cx, float cy, float fx,
    float fy, float r, uint8_t opacity) {
    return Paint(std::make_shared<paint::RadialGradient>(ramp_ptr,
        cx, cy, fx, fy, r), opacity);
}

Window window(float xl, float yb, float xr, float yt) {
    return Window(xl, yb, xr, yt);
}

Viewport viewport(int xl, int yb, int xr, int yt) {
    return Viewport(xl, yb, xr, yt);
}

namespace cap {
    stroke::Cap butt = stroke::Cap::butt;
    stroke::Cap round = stroke::Cap::round;
    stroke::Cap square = stroke::Cap::square;
    stroke::Cap triangle = stroke::Cap::triangle;
    stroke::Cap fletching = stroke::Cap::fletching;
}

namespace join {
    stroke::Join arcs = stroke::Join::arcs;
    stroke::Join miter = stroke::Join::miter;
    stroke::Join miterclip = stroke::Join::miterclip;
    stroke::Join round = stroke::Join::round;
    stroke::Join bevel = stroke::Join::bevel;
}

namespace method {
    stroke::Method curves = stroke::Method::curves;
    stroke::Method lines = stroke::Method::lines;
    stroke::Method driver = stroke::Method::driver;
}

namespace spread {
    paint::Spread repeat = paint::Spread::repeat;
    paint::Spread reflect = paint::Spread::reflect;
    paint::Spread pad = paint::Spread::pad;
    paint::Spread transparent = paint::Spread::transparent;
}

} } // namespace rvg::description
