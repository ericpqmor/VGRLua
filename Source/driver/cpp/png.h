#ifndef RVG_DRIVER_PNG_H
#define RVG_DRIVER_PNG_H

#include <vector>
#include <string>
#include <cstdio>

#include "bbox/viewport.h"
#include "scene/xformablescene.h"

namespace rvg {
    namespace driver {
        namespace png {

using rvg::scene::XformableScene;
using rvg::bbox::Viewport;

// Redefine this to be your own acceleration datastructure
using Accelerated = XformableScene;

// Builds the acceleration datastructure from a scene and a viewport
Accelerated accelerate(const XformableScene &xs, const Viewport &vp);

// Uses the acceleration datastructure to render scene into viewport
void render(const Accelerated &accel, const Viewport &vp,
    FILE *out, const std::vector<std::string> &args =
        std::vector<std::string>());

} } } // namespace rvg::driver::png

#endif
