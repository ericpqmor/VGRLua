#include "path.h"

namespace rvg {
    namespace path {

void Path::clear(void) {
    m_instructions.clear();
    m_offsets.clear();
    m_data.clear();
}

void Path::do_linear_segment(float x0, float y0, float x1, float y1) {
    (void) x0; (void) y0; // ignored, comes from previous instruction
    push_instruction(Instruction::linear_segment);
    push_data(x1, y1);
}

void Path::do_linear_segment_with_length(float x0, float y0, float len,
    float x1, float y1) {
    (void) x0; (void) y0; // ignored, comes from previous instruction
    push_instruction(Instruction::linear_segment_with_length);
    push_data(len, x1, y1);
}

void Path::do_degenerate_segment(float x0, float y0, float dx0, float dy0,
        float dx1, float dy1, float x1, float y1) {
    (void) x0; (void) y0; // ignored, comes from previous instruction
    push_instruction(Instruction::degenerate_segment);
    push_data(dx0, dy0, dx1, dy1, x1, y1);
}

void Path::do_quadratic_segment(float x0, float y0, float x1, float y1,
    float x2, float y2) {
    (void) x0; (void) y0; // ignored, comes from previous instruction
    push_instruction(Instruction::quadratic_segment);
    push_data(x1, y1, x2, y2);
}

void Path::do_rational_quadratic_segment(float x0, float y0, float x1, float y1,
    float w1, float x2, float y2) {
    (void) x0; (void) y0; // ignored, comes from previous instruction
    push_instruction(Instruction::rational_quadratic_segment);
    push_data(x1, y1, w1, x2, y2);
}

void Path::do_cubic_segment(float x0, float y0, float x1, float y1,
    float x2, float y2, float x3, float y3) {
    (void) x0; (void) y0; // ignored, comes from previous instruction
    push_instruction(Instruction::cubic_segment);
    push_data(x1, y1, x2, y2, x3, y3);
}

void Path::do_begin_segment(float ddx, float ddy, float ddw,
            float dx, float dy, float dw, float x, float y) {
    push_instruction(Instruction::begin_segment, 0);
    push_data(ddx, ddy, ddw, dx, dy, dw, x, y);
}

void Path::do_end_segment(float ddx, float ddy, float ddw,
            float dx, float dy, float dw, float x, float y) {
    push_instruction(Instruction::end_segment, 0);
    push_data(ddx, ddy, ddw, dx, dy, dw, x, y);
}

void Path::do_begin_open_contour(int len, float x0, float y0) {
    begin_group(Instruction::begin_open_contour, len, x0, y0);
}

void Path::do_end_open_contour(float x0, float y0, int len) {
    end_group(Instruction::end_open_contour, x0, y0, len);
}

void Path::do_begin_closed_contour(int len, float x0, float y0) {
    begin_group(Instruction::begin_closed_contour, len, x0, y0);
}

void Path::do_end_closed_contour(float x0, float y0, int len) {
    end_group(Instruction::end_closed_contour, x0, y0, len);
}

void Path::do_begin_dash(int len, float x0, float y0) {
    begin_group(Instruction::begin_dash, len, x0, y0);
}

void Path::do_end_dash(float x0, float y0, int len) {
    end_group(Instruction::end_dash, x0, y0, len);
}

uint32_t Path::do_get_data_size(void) const {
    return static_cast<uint32_t>(m_data.size());
}

uint32_t Path::do_get_instructions_size(void) const {
    return static_cast<uint32_t>(m_instructions.size());
}

uint32_t Path::do_get_offsets_size(void) const {
    return static_cast<uint32_t>(m_offsets.size());
}

uint32_t Path::do_get_offset(uint32_t index) const {
    return m_offsets[index];
}

void Path::do_set_datum(uint32_t offset, const Datum &datum) {
    m_data[offset] = datum;
}

void Path::do_set_instruction(uint32_t index, const Instruction &instruction) {
    m_instructions[index] = instruction;
}

void Path::begin_group(Instruction type, int len, float x0, float y0) {
    push_instruction(type, 0);
    push_data(len, x0, y0);
}

void Path::end_group(Instruction type, float x0, float y0, int len) {
    (void) x0; (void) y0; // ignored, comes from previous instruction
    push_instruction(type);
    push_data(len);
}

void Path::push_instruction(Instruction instruction, int rewind) {
    m_instructions.push_back(instruction);
    m_offsets.push_back(static_cast<uint32_t>(m_data.size()+rewind));
}

void Path::push_data(void) {
    ;
}

} } // namespace rvg::path
