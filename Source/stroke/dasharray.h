#ifndef RVG_STROKE_DASHARRAY_H
#define RVG_STROKE_DASHARRAY_H

#include <vector>
#include <memory>

namespace rvg {
    namespace stroke {

using DashArray = std::vector<float>;
using DashArrayPtr = std::shared_ptr<DashArray>;

} } // namespace rvg::stroke

#endif
