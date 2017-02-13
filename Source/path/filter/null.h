#ifndef RVG_PATH_FILTER_NULL_H
#define RVG_PATH_FILTER_NULL_H

#include "path/ipath.h"

namespace rvg {
    namespace path {
        namespace filter {

class Null final: public IPath<Null> {
private:
    friend IPath<Null>;
    void do_linear_segment(float, float, float, float) { ; }
    void do_linear_segment_with_length(float, float, float, float, float) { ; }
    void do_degenerate_segment(float, float, float, float, float, float,
        float, float) { ; }
    void do_quadratic_segment(float, float, float, float, float, float) { ; }
    void do_rational_quadratic_segment(float, float, float, float, float,
        float, float) { ; }
    void do_cubic_segment(float, float, float, float, float, float,
        float, float) { ; }
    void do_begin_segment(float, float, float, float, float, float, float, float) { ; }
    void do_end_segment(float, float, float, float, float, float, float, float) { ; }
    void do_begin_open_contour(int, float, float) { ; }
    void do_end_open_contour(float, float, int) { ; }
    void do_begin_closed_contour(int, float, float) { ; }
    void do_end_closed_contour(float, float, int) { ; }
    void do_begin_dash(int, float, float) { ; }
    void do_end_dash(float, float, int) { ; }
    uint32_t do_get_data_size(void) const { return 0; }
    uint32_t do_get_instructions_size(void) const { return 0; }
    uint32_t do_get_offsets_size(void) const { return 0; }
    uint32_t do_get_offset(uint32_t) const { return 0; }
    void do_set_datum(uint32_t, const Datum &) { ; }
    void do_set_instruction(uint32_t, const Instruction &) { ; }
};

} } } // namespace rvg::path::filter

#endif
