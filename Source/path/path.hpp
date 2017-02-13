template <typename PF>
void Path::iterate(PF &forward, int first, int last) const {
    first = std::max(0, first);
    last = std::min(static_cast<int>(m_instructions.size()), last);
    using i = Instruction;
    for (int index = first; index < last; ++index) {
        auto offset = m_offsets[index];
        switch (m_instructions[index]) {
            case i::begin_open_contour:
                forward.begin_open_contour(m_data[offset],
                    m_data[offset+1], m_data[offset+2]);
                break;
            case i::end_open_contour:
                forward.end_open_contour(m_data[offset],
                    m_data[offset+1], m_data[offset+2]);
                break;
            case i::begin_closed_contour:
                forward.begin_closed_contour(m_data[offset],
                    m_data[offset+1], m_data[offset+2]);
                break;
            case i::end_closed_contour:
                forward.end_closed_contour(m_data[offset],
                    m_data[offset+1], m_data[offset+2]);
                break;
            case i::degenerate_segment:
                forward.degenerate_segment(m_data[offset],
                    m_data[offset+1], m_data[offset+2], m_data[offset+3],
                    m_data[offset+4], m_data[offset+5], m_data[offset+6],
                    m_data[offset+7]);
                break;
            case i::linear_segment:
                forward.linear_segment(m_data[offset],
                    m_data[offset+1], m_data[offset+2], m_data[offset+3]);
                break;
            case i::linear_segment_with_length:
                forward.linear_segment_with_length(m_data[offset],
                    m_data[offset+1], m_data[offset+2], m_data[offset+3],
                    m_data[offset+4]);
                break;
            case i::quadratic_segment:
                forward.quadratic_segment(m_data[offset],
                    m_data[offset+1], m_data[offset+2], m_data[offset+3],
                    m_data[offset+4], m_data[offset+5]);
                break;
            case i::rational_quadratic_segment:
                forward.rational_quadratic_segment(m_data[offset],
                    m_data[offset+1], m_data[offset+2], m_data[offset+3],
                    m_data[offset+4], m_data[offset+5], m_data[offset+6]);
                break;
            case i::cubic_segment:
                forward.cubic_segment(m_data[offset],
                    m_data[offset+1], m_data[offset+2], m_data[offset+3],
                    m_data[offset+4], m_data[offset+5], m_data[offset+6],
                    m_data[offset+7]);
                break;
            case i::begin_segment:
                forward.begin_segment(m_data[offset],
                    m_data[offset+1], m_data[offset+2], m_data[offset+3],
                    m_data[offset+4], m_data[offset+5], m_data[offset+6],
                    m_data[offset+7]);
                break;
            case i::end_segment:
                forward.end_segment(m_data[offset],
                    m_data[offset+1], m_data[offset+2], m_data[offset+3],
                    m_data[offset+4], m_data[offset+5], m_data[offset+6],
                    m_data[offset+7]);
                break;
            case i::begin_dash:
                forward.begin_dash(m_data[offset],
                    m_data[offset+1], m_data[offset+2]);
                break;
            case i::end_dash:
                forward.end_dash(m_data[offset],
                    m_data[offset+1], m_data[offset+2]);
                break;
        }
    }
}

template <typename PF>
void Path::iterate(PF &forward) const {
    this->iterate(forward, 0, static_cast<int>(m_instructions.size()));
}

template <typename ...REST>
void Path::push_data(Datum first, REST ...rest) {
    m_data.push_back(first);
    push_data(rest...);
}
