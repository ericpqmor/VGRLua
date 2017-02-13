#ifndef RVG_PATH_FILTER_FORWARDER_H
#define RVG_PATH_FILTER_FORWARDER_H

#include <utility>     // std::forward

#include "path/ipath.h"

namespace rvg {
    namespace path {
        namespace filter {

template <typename DERIVED, typename SINK>
class Forwarder: public IPath<DERIVED> {

    SINK m_sink;

public:
    explicit Forwarder(SINK &&sink): m_sink(std::forward<SINK>(sink)) {
        static_assert(rvg::meta::is_an_ipath<SINK>::value,
            "sink is not an IPath");
    }

protected:
    friend IPath<DERIVED>;

    void do_linear_segment(float x0, float y0, float x1, float y1) {
        return m_sink.linear_segment(x0, y0, x1, y1);
    }

    void do_linear_segment_with_length(float x0, float y0, float len,
        float x1, float y1) {
        return m_sink.linear_segment_with_length(x0, y0, len, x1, y1);
    }

    void do_degenerate_segment(float x0, float y0, float dx0, float dy0,
            float dx1, float dy1, float x1, float y1) {
        return m_sink.degenerate_segment(x0, y0, dx0, dy0, dx1, dy1, x1, y1);
    }

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2) {
        return m_sink.quadratic_segment(x0, y0, x1, y1, x2, y2);
    }

    void do_rational_quadratic_segment(float x0, float y0,
        float x1, float y1, float w1, float x2, float y2) {
        return m_sink.rational_quadratic_segment(x0, y0, x1, y1, w1, x2, y2);
    }

    void do_cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3) {
        return m_sink.cubic_segment(x0, y0, x1, y1, x2, y2, x3, y3);
    }

    void do_begin_segment(float ddx, float ddy, float ddw,
            float dx, float dy, float dw, float x, float y) {
        return m_sink.begin_segment(ddx, ddy, ddw, dx, dy, dw, x, y);
    }

    void do_end_segment(float ddx, float ddy, float ddw,
            float dx, float dy, float dw, float x, float y) {
        return m_sink.end_segment(ddx, ddy, ddw, dx, dy, dw, x, y);
    }

    void do_begin_open_contour(int len, float x0, float y0) {
        return m_sink.begin_open_contour(len, x0, y0);
    }

    void do_end_open_contour(float x0, float y0, int len) {
        return m_sink.end_open_contour(x0, y0, len);
    }

    void do_begin_closed_contour(int len, float x0, float y0) {
        return m_sink.begin_closed_contour(len, x0, y0);
    }

    void do_end_closed_contour(float x0, float y0, int len) {
        return m_sink.end_closed_contour(x0, y0, len);
    }

    void do_begin_dash(int len, float x0, float y0) {
        return m_sink.begin_dash(len, x0, y0);
    }

    void do_end_dash(float x0, float y0, int len) {
        return m_sink.end_dash(x0, y0, len);
    }

    uint32_t do_get_data_size(void) const {
        return m_sink.get_data_size();
    }

    uint32_t do_get_instructions_size(void) const {
        return m_sink.get_instructions_size();
    }

    uint32_t do_get_offsets_size(void) const {
        return m_sink.get_offsets_size();
    }

    uint32_t do_get_offset(uint32_t index) const {
        return m_sink.get_offset(index);
    }

    void do_set_datum(uint32_t offset, const Datum &datum) {
        return m_sink.set_datum(offset, datum);
    }

    void do_set_instruction(uint32_t index,
        const Instruction &instruction) {
        m_sink.set_instruction(index, instruction);
    }

};

template <typename SINK, typename DERIVED>
Forwarder<SINK, DERIVED> make_forwarder(SINK &&sink) {
    return Forwarder<SINK, DERIVED>(std::forward<SINK>(sink));
}

} } } // namespace rvg::path::filter

#endif
