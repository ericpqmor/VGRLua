#include <iostream>

#include "stroke/style.h"

namespace rvg {
    namespace stroke {

const DashArrayPtr
    Style::default_dash_array_ptr = std::make_shared<DashArray>();

std::ostream &operator<<(std::ostream &out, const Cap cap) {
    switch (cap) {
        case Cap::butt:
            out << "butt";
            break;
        case Cap::round:
            out << "round";
            break;
        case Cap::square:
            out << "square";
            break;
        case Cap::triangle:
            out << "triangle";
            break;
        case Cap::fletching:
            out << "fletching";
            break;
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const Join join) {
    switch (join) {
        case Join::arcs:
            out << "arcs";
            break;
        case Join::miter:
            out << "miter";
            break;
        case Join::miterclip:
            out << "miterclip";
            break;
        case Join::round:
            out << "round";
            break;
        case Join::bevel:
            out << "bevel";
            break;
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const Method method) {
    switch (method) {
        case Method::curves:
            out << "curves";
            break;
        case Method::lines:
            out << "lines";
            break;
        case Method::driver:
            out << "driver";
            break;
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const Style &st) {
    out << "stroked(" << st.width() << ')';
    if (st.join() != Style::default_join ||
        st.miter_limit() != Style::default_miter_limit) {
        out << ".joined(join::" << st.join();
        if (st.miter_limit() != Style::default_miter_limit) {
             out << ',' << st.miter_limit();
        }
        out << ')';
    }
    if (st.cap() != Style::default_cap) {
        out << ".capped(cap::" << st.cap() << ")";
    }
    if (!st.dash_array().empty()) {
        out << ".dashed({";
        for (float d: st.dash_array()) out << d << ",";
        out << "}";
        if (st.phase_reset() != Style::default_phase_reset ||
            st.initial_phase() != Style::default_initial_phase) {
            out << ',' << st.initial_phase() << ',' << st.phase_reset();
        }
        out << ')';
    }
    if (st.method() != Style::default_method) {
        out << ".by(method::" << st.method() << ')';
    }
    return out;
}

} } // namespace rvg::stroke
