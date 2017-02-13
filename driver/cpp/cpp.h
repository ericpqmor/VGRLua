#ifndef RVG_DRIVER_CPP_H
#define RVG_DRIVER_CPP_H

#include <iosfwd>
#include <string>
#include <vector>

#include "description/description.h"
#include "bbox/viewport.h"

namespace rvg {
    namespace driver {
        namespace cpp {

using rvg::description::XformableScene;
using rvg::bbox::Viewport;

using Accelerated = XformableScene;

Accelerated accelerate(const XformableScene &xs, const Viewport &vp);

void render(const Accelerated &accel, const Viewport &vp,
    std::ostream &out, const std::vector<std::string> &args =
        std::vector<std::string>());

} } } // namespace rvg::driver::cpp

#endif
