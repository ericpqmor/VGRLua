#ifndef RVG_PATH_DATUM_H
#define RVG_PATH_DATUM_H

#include <cstdint>

namespace rvg {
    namespace path {

// This represents a float or a 32-bit integer
union Datum {
    Datum(float ff ) {  f = ff; }
    Datum(int32_t ii) {  i = ii; }
    operator int() const { return i; }
    operator float() const { return f; }
    float f;
    int32_t i;
};

} } // namespace rvg::path

#endif
