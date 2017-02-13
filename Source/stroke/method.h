#ifndef RVG_STROKE_METHOD_H
#define RVG_STROKE_METHOD_H

#include <iosfwd>
#include <cstdint>

namespace rvg {
    namespace stroke {

enum class Method: uint8_t {
    curves,
    lines,
    driver,
};

std::ostream &operator<<(std::ostream &out, const Method method);

} } // namespace rvg::stroke

#endif
