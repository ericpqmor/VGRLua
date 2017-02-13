#ifndef RVG_PATH_IPATH_H
#define RVG_PATH_IPATH_H

#include <vector>
#include <iostream>
#include <cassert>
#include <cctype>
#include <algorithm>
#include <memory>

#include "meta/meta.h"
#include "path/datum.h"
#include "path/instruction.h"

namespace rvg {
    namespace path {

template <typename DERIVED>
class IPath {
protected:
    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:
    void linear_segment(float x0, float y0, float x1, float y1) {
        return derived().do_linear_segment(x0, y0, x1, y1);
    }

    void linear_segment_with_length(float x0, float y0, float len,
        float x1, float y1) {
        return derived().do_linear_segment_with_length(x0, y0, len, x1, y1);
    }

    void degenerate_segment(float x0, float y0, float dx0, float dy0,
            float dx1, float dy1, float x1, float y1) {
        return derived().do_degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1);
    }

    void quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
        return derived().do_quadratic_segment(x0, y0, x1, y1, x2, y2);
    }

    void rational_quadratic_segment(float x0, float y0, float x1, float y1,
        float w1, float x2, float y2) {
        return derived().do_rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2);
    }

    void cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
        return derived().do_cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3);
    }

    void begin_segment(float ddx, float ddy, float ddw,
            float dx, float dy, float dw, float x, float y) {
        return derived().do_begin_segment(ddx, ddy, ddw, dx, dy, dw, x, y);
    }

    void end_segment(float ddx, float ddy, float ddw,
            float dx, float dy, float dw, float x, float y) {
        return derived().do_end_segment(ddx, ddy, ddw, dx, dy, dw, x, y);
    }

    void begin_open_contour(int len, float x0, float y0) {
        return derived().do_begin_open_contour(len, x0, y0);
    }

    void end_open_contour(float x0, float y0, int len) {
        return derived().do_end_open_contour(x0, y0, len);
    }

    void begin_closed_contour(int len, float x0, float y0) {
        return derived().do_begin_closed_contour(len, x0, y0);
    }

    void end_closed_contour(float x0, float y0, int len) {
        return derived().do_end_closed_contour(x0, y0, len);
    }

    void begin_dash(int len, float x0, float y0) {
        return derived().do_begin_dash(len, x0, y0);
    }

    void end_dash(float x0, float y0, int len) {
        return derived().do_end_dash(x0, y0, len);
    }

    uint32_t get_data_size(void) const {
        return static_cast<const DERIVED *>(this)->do_get_data_size();
    }

    uint32_t get_instructions_size(void) const {
        return static_cast<const DERIVED *>(this)->do_get_instructions_size();
    }

    uint32_t get_offsets_size(void) const {
        return static_cast<const DERIVED *>(this)->do_get_offsets_size();
    }

    uint32_t get_offset(uint32_t index) const {
        return static_cast<const DERIVED *>(this)->do_get_offset(index);
    }

    void set_datum(uint32_t offset, const Datum &datum) {
        return derived().do_set_datum(offset, datum);
    }

    void set_instruction(uint32_t index, const Instruction &instruction) {
        return derived().do_set_instruction(index, instruction);
    }
};

} } // namespace rvg::path

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_ipath = std::integral_constant<
    bool,
    is_crtp_of<
        rvg::path::IPath,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // namespace rvg::meta

#endif
