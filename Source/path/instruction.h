#ifndef RVG_PATH_INSTRUCTION_H
#define RVG_PATH_INSTRUCTION_H

#include <cstdint>

namespace rvg {
    namespace path {

// These are the instructions that live in a path
enum class Instruction: uint8_t {
    begin_open_contour,
    end_open_contour,
    begin_closed_contour,
    end_closed_contour,
    degenerate_segment,
    linear_segment,
    linear_segment_with_length,
    quadratic_segment,
    rational_quadratic_segment,
    cubic_segment,
    begin_segment,
    end_segment,
    begin_dash,
    end_dash
};

} } // namespace rvg::path

#endif
