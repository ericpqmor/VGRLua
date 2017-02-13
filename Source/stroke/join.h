#ifndef RVG_STROKE_JOIN_H
#define RVG_STROKE_JOIN_H

#include <iosfwd>
#include <cstdint>

namespace rvg {
    namespace stroke {

enum class Join: uint8_t {
    arcs,
    miter,
    miterclip,
    round,
    bevel
};

std::ostream &operator<<(std::ostream &out, const Join join);

} } // namespace rvg::stroke

#endif
