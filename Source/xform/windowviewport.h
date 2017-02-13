#ifndef RVG_XFORM_WINDOWVIEWPORT_H
#define RVG_XFORM_WINDOWVIEWPORT_H

#include "bbox/window.h"
#include "bbox/viewport.h"

namespace rvg {
    namespace xform {

enum class Align {
    min,  // align min corners of window and viewport
    mid,  // align centers of window and viewport
    max,  // align max corners of window and viewport
};

enum class Aspect {
    none,   // do not preserve aspect ratio, blindly map window to viewport
    extend, // extend window to smallest rectangle with same aspect as viewport
    trim    // trim window to largest rectangle with same aspect as viewport
};

Affinity windowviewport(const rvg::bbox::Window &window,
    const rvg::bbox::Viewport &viewport, Align align_x = Align::mid,
    Align align_y = Align::mid, Aspect aspect = Aspect::none);

} } // namespace rvg::xform

#endif
