#ifndef RVG_PATH_H
#define RVG_PATH_H

#include <vector>
#include <iostream>
#include <cassert>
#include <cctype>
#include <algorithm>
#include <memory>

#include "path/datum.h"
#include "path/instruction.h"
#include "path/ipath.h"

namespace rvg {
    namespace path {

// Our internal representation for a Path consists of an array
// with instructions, an array of offsets, and an array with
// data.  Each instruction has a corresponding offset entry
// pointing into the data array.
//
// Contours are bracketed by a begin/end pair of
// instructions.  The pair can be either open or closed.
// (This is related to closepath or Z). During the dashing
// and offsetting process, each original segment is
// bracketed by a begin/end instruction pair, and each dash
// is also bracketed by a begin/end instrution pair.
// Contour and dashing bracketing instructions carry a
// length data that represents the number to be added to the
// begin instruction index to reach the end instruction
// index.
//
// The offset of each instruction points to
// the start of the instruction's data, so that all
// instructions can be processed in parallel if need be.
// Many instructions share common data. In the table below,
// the data that each instruction needs when being added to
// a path is marked with '+'. The data to which the
// instruction's offset points is marked with a '^'
//
// BOC ^+len +x0 +y0                      ; begin_open_contour
// EOC ^x0 y0 +len                        ; end_open_contour
// BCC ^+len +x0 +y0                      ; begin_closed_contour
// ECC ^x0 y0 +len                        ; end_closed_contour
// DS  ^x0 y0 +dx0 +dy0 +dx1 +dy1 +x1 +y1 ; degenerate_segment
// LS  ^x0 y0 +x1 +y1                     ; linear_segment
// QS  ^x0 y0 +x1 +y1 +x2 +y2             ; quadratic_segment
// RQS ^x0 y0 +x1 +y1 +w1 +x2 +y2         ; rational_quadratic_segment
// CS  ^x0 y0 +x1 +y1 +x2 +y2 +x3 +y3     ; cubic_segment
//
// The degenerate segment represents a segment with zero or
// very small length. dx0 dy0 represents the tangent
// direction before the segment. dx1 dy1 the tangent
// direction after the segment. x1 y1 is the endpoint, which
// may be repeated
//
// The len in begin/end instructions (when applicable) that allows us
// to find the matching end/begin instruction can be computed
// automatically by means of a helper Bracketer class.
//
// The idea is that the representation is reversible in the
// sense that traversing it forward or backward is equally
// easy. The datastructure also provide easy random access
// to the data for each instruction
//
// The internal
// representation can be used to directly add instructions
// and associated data, without much in the way of
// consistency checks.
//
// There is also an SVGForwarder helper class that can take SVG
// commands, convert them to our instructions, and forward
// them to a Path object.

// Main path class. It simply stores the data
class Path final: public IPath<Path> {
private:
    std::vector<Instruction> m_instructions;
    std::vector<uint32_t> m_offsets;
    std::vector<Datum> m_data;

public:

    // default copy and move constructor and assignment operators
    Path() = default;
    Path(const Path &other) = default;
    Path(Path &&other) = default;
    Path &operator=(const Path &other) = default;
    Path &operator=(Path &&other) = default;

    template <typename PF>
    void iterate(PF &forward, int first, int last) const;

    template <typename PF>
    void iterate(PF &&forward, int first, int last) const {
        this->iterate(forward, first, last);
    }

    template <typename PF>
    void iterate(PF &forward) const;

    template <typename PF>
    void iterate(PF &&forward) const {
        this->iterate(forward);
    }

    void clear(void);

protected:

    friend IPath<Path>;

    void do_linear_segment(float x0, float y0, float x1, float y1);

    void do_linear_segment_with_length(float x0, float y0, float len,
        float x1, float y1);

    void do_degenerate_segment(float x0, float y0, float dx0, float dy0,
            float dx1, float dy1, float x1, float y1);

    void do_quadratic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2);

    void do_rational_quadratic_segment(float x0, float y0, float x1, float y1,
        float w1, float x2, float y2);

    void do_cubic_segment(float x0, float y0, float x1, float y1,
        float x2, float y2, float x3, float y3);

    void do_begin_segment(float ddx, float ddy, float ddw,
            float dx, float dy, float dw, float x, float y);

    void do_end_segment(float ddx, float ddy, float ddw,
            float dx, float dy, float dw, float x, float y);

    void do_begin_open_contour(int len, float x0, float y0);

    void do_end_open_contour(float x0, float y0, int len);

    void do_begin_closed_contour(int len, float x0, float y0);

    void do_end_closed_contour(float x0, float y0, int len);

    void do_begin_dash(int len, float x0, float y0);

    void do_end_dash(float x0, float y0, int len);

    uint32_t do_get_data_size(void) const;

    uint32_t do_get_instructions_size(void) const;

    uint32_t do_get_offsets_size(void) const;

    uint32_t do_get_offset(uint32_t index) const;

    void do_set_datum(uint32_t offset, const Datum &datum);

    void do_set_instruction(uint32_t index, const Instruction &instruction);

    void begin_group(Instruction type, int len, float x0, float y0);

    void end_group(Instruction type, float x0, float y0, int len);

    void push_instruction(Instruction instruction, int rewind = -2);

    template <typename ...REST>
    void push_data(Datum first, REST ...rest);

    void push_data(void);

};

#include "path/path.hpp"

using PathPtr = std::shared_ptr<Path>;

} } // namespace rvg::path

#endif
