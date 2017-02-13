#ifndef RVG_PATH_FILTER_BRACKET_LENGTHS_H
#define RVG_PATH_FILTER_BRACKET_LENGTHS_H

#include <type_traits>
#include <utility>

#include "path/filter/forwarder.h"

namespace rvg {
    namespace path {
        namespace filter {

template <typename SINK>
class BracketLengths final: public Forwarder<BracketLengths<SINK>, SINK> {
public:
    using Base = Forwarder<BracketLengths<SINK>, SINK>;

    explicit BracketLengths(SINK &&sink):
    Base(std::forward<SINK>(sink)), m_begin_contour(-1), m_begin_dash(-1) {
        static_assert(rvg::meta::is_an_ipath<SINK>::value,
            "sink is not an IPath");
    }

private:

    friend IPath<BracketLengths<SINK>>;

    void do_begin_dash(int len, float x0, float y0) {
        (void) len; // ignored, recomputed
        assert(m_begin_dash < 0); // no nested dashes
        Base::do_begin_dash(0, x0, y0);
        m_begin_dash = Base::do_get_instructions_size();
    }

    void do_end_dash(float x0, float y0, int len) {
        assert(m_begin_contour >= 0); // unfinished contour
        assert(m_begin_dash >= 0); // unfinished dash
        Base::do_end_dash(x0, y0, 0);
        int end_dash = Base::do_get_instructions_size() - 1;
        len = end_dash - m_begin_dash;
        Base::do_set_datum(Base::do_get_offset(m_begin_dash), len);
        Base::do_set_datum(Base::do_get_offset(end_dash)+2, len);
    }

    void do_begin_closed_contour(int len, float x0, float y0) {
        (void) len; // ignored, recomputed
        assert(m_begin_dash < 0); // no unfinished dashes
        assert(m_begin_contour < 0); // no nested contours
        Base::do_begin_closed_contour(0, x0, y0);
        m_begin_contour = Base::do_get_instructions_size() - 1;
    }

    void do_begin_open_contour(int len, float x0, float y0) {
        (void) len; // ignored, recomputed
        assert(m_begin_dash < 0); // no unfinished dashes
        assert(m_begin_contour < 0); // no nested contours
        Base::do_begin_open_contour(0, x0, y0);
        m_begin_contour = Base::do_get_instructions_size() - 1;
    }

    void do_end_closed_contour(float x0, float y0, int len) {
        assert(m_begin_dash < 0); // no unfinished dashes
        assert(m_begin_contour >= 0); // unfinished contour
        Base::do_set_instruction(m_begin_contour,
            Instruction::begin_closed_contour);
        Base::do_end_closed_contour(x0, y0, 0);
        int end_contour = Base::do_get_instructions_size() - 1;
        len = end_contour - m_begin_contour;
        Base::do_set_datum(Base::do_get_offset(m_begin_contour), len);
        Base::do_set_datum(Base::do_get_offset(end_contour)+2, len);
        m_begin_contour = -1;
    }

    void do_end_open_contour(float x0, float y0, int len) {
        assert(m_begin_dash < 0); // no unfinished dashes
        assert(m_begin_contour >= 0); // unfinished contour
        Base::do_set_instruction(m_begin_contour,
            Instruction::begin_open_contour);
        Base::do_end_open_contour(x0, y0, 0);
        int end_contour = Base::do_get_instructions_size() - 1;
        len = end_contour - m_begin_contour;
        Base::do_set_datum(Base::do_get_offset(m_begin_contour), len);
        Base::do_set_datum(Base::do_get_offset(end_contour)+2, len);
        m_begin_contour = -1;
    }

private:
    int m_begin_contour;
    int m_begin_dash;
};

template <typename SINK>
BracketLengths<SINK> make_bracket_lengths(SINK &&sink) {
    return BracketLengths<SINK>(std::forward<SINK>(sink));
}

} } } // namespace rvg::path::filter

#endif
