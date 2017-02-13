#ifndef RVG_PAINT_SPREAD_H
#define RVG_PAINT_SPREAD_H

#include <iosfwd>

namespace rvg {
    namespace paint {

enum class Spread {
    pad,
    repeat,
    reflect,
    transparent
};

std::ostream &operator<<(std::ostream &out, const Spread spread);

} } // namespace rvg::paint

#endif
