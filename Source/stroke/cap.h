#ifndef RVG_STROKE_CAP_H
#define RVG_STROKE_CAP_H

#include <iosfwd>
#include <cstdint>

namespace rvg {
    namespace stroke {

enum class Cap: uint8_t {
    butt,
    round,
    square,
    triangle,
    fletching
};

std::ostream &operator<<(std::ostream &out, const Cap cap);

} } // namespace rvg::stroke

#endif
