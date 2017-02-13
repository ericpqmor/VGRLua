#ifndef RVG_DESCRIPTION_H
#define RVG_DESCRIPTION_H

#include <vector>

#include "path/svg/command.h"
#include "shape/shape.h"
#include "xform/xform.h"
#include "bbox/window.h"
#include "bbox/viewport.h"
#include "color/color.h"
#include "color/named.h"
#include "stroke/cap.h"
#include "stroke/join.h"
#include "stroke/method.h"
#include "paint/spread.h"
#include "image/iimage.h"
#include "image/pngio.h"
#include "base64/base64.h"
#include "scene/xformablescene.h"
#include "description/painted.h"
#include "description/stencil.h"

namespace rvg {
    namespace description {

using ILPainted = std::initializer_list<Painted>;
using VPainted = std::vector<Painted>;
using ILStencil = std::initializer_list<Stencil>;
using VStencil = std::vector<Stencil>;
using ILToken = std::initializer_list<path::svg::Token>;

using xform::Xform;
using shape::Shape;
using paint::Paint;
using scene::XformableScene;
using scene::Scene;
using paint::Ramp;
using paint::RampPtr;
using paint::Spread;
using paint::Stop;
using image::IImagePtr;
using bbox::Window;
using bbox::Viewport;

using namespace path::svg::command;

namespace base64 {
    using rvg::base64::decode;
}

namespace color {
    using namespace rvg::color::named;
}

namespace image {
    namespace png {
    using rvg::image::pngio::load;
} }

namespace cap {
    extern stroke::Cap butt;
    extern stroke::Cap round;
    extern stroke::Cap square;
    extern stroke::Cap triangle;
    extern stroke::Cap fletching;
}

namespace join {
    extern stroke::Join arcs;
    extern stroke::Join miter;
    extern stroke::Join miterclip;
    extern stroke::Join round;
    extern stroke::Join bevel;
}

namespace method {
    extern stroke::Method curves;
    extern stroke::Method lines;
    extern stroke::Method driver;
}

namespace spread {
    extern paint::Spread repeat;
    extern paint::Spread reflect;
    extern paint::Spread pad;
    extern paint::Spread transparent;
}

using rvg::color::rgba;
using rvg::color::rgb;
using rvg::color::rgba8;
using rvg::color::rgb8;

using xform::rotation;
using xform::translation;
using xform::scaling;
using xform::windowviewport;

Shape path(const char *svg);
Shape path(const ILToken &svg);

Shape polygon(const std::initializer_list<float> &point_list);
Shape circle(float x, float y, float r);
Shape triangle(float x1, float y1, float x2, float y2, float x3, float y3);
Shape rect(float x, float y, float width, float height);

Painted nzfill(const Shape &shape, const Paint &paint);
Painted zfill(const Shape &shape, const Paint &paint);
Painted ofill(const Shape &shape, const Paint &paint);
Painted efill(const Shape &shape, const Paint &paint);
Painted fill(const Shape &shape, const Paint &paint);
Painted eofill(const Shape &shape, const Paint &paint);

Painted transparentize(float opacity, const VPainted &painted);
Painted transparentize(float opacity, const ILPainted &painted);

Painted transform(const Xform &xf, const ILPainted &painted);
Painted transform(const Xform &xf, const VPainted &painted);

Painted blur(float radius, const Painted &painted);
Painted blur(float radius, const VPainted &painted);

Painted fade(uint8_t opacity, const Painted &painted);
Painted fade(uint8_t opacity, const VPainted &painted);

Stencil nzstencil(const Shape &shape);
Stencil zstencil(const Shape &shape);
Stencil ostencil(const Shape &shape);
Stencil estencil(const Shape &shape);
Stencil stencil(const Shape &shape);
Stencil eostencil(const Shape &shape);

Painted clip(const VStencil &clipper, const VPainted &clipped);
Painted clip(const ILStencil &clipper, const ILPainted &clippee);
Painted clip(const Stencil &clipper, const ILPainted &clippee);
Painted clip(const ILStencil &clipper, const Painted &clippee);
Painted clip(const Stencil &clipper, const Painted &clippee);

Stencil clip(const VStencil &clipper, const VStencil &clipped);
Stencil clip(const ILStencil &clipper, const ILStencil &clippee);
Stencil clip(const Stencil &clipper, const ILStencil &clippee);
Stencil clip(const ILStencil &clipper, const Stencil &clippee);
Stencil clip(const Stencil &clipper, const Stencil &clippee);

Stencil transform(const Xform &xf, const VStencil &stencil);
Stencil transform(const Xform &xf, const ILStencil &stencil);

Window window(float xl, float yb, float xr, float yt);
Viewport viewport(int xl, int yb, int xr, int yt);

XformableScene scene(const Painted &painted);
XformableScene scene(const ILPainted &painted);
XformableScene scene(const std::vector<Painted> &painted);

Paint solid_color(const rvg::color::RGBA8 &color, uint8_t opacity = 255);

RampPtr ramp(Spread spread, const std::initializer_list<Stop> &stops);
RampPtr ramp(const std::initializer_list<Stop> &stops);

Paint linear_gradient(const RampPtr &ramp_ptr, float x1, float y1,
    float x2, float y2, uint8_t opacity = 255);

Paint radial_gradient(const RampPtr &ramp_ptr, float cx, float cy,
    float fx, float fy, float r, uint8_t opacity = 255);

Paint texture(Spread spread, const IImagePtr &image_ptr, uint8_t opacity = 255);
Paint texture(const IImagePtr &image_ptr, uint8_t opacity = 255);

struct Description {
    XformableScene scene;
    Window window;
    Viewport viewport;
};

} } // namespace rvg::description

#endif
