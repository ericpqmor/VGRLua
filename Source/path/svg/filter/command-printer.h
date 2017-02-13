#ifndef PATH_SVG_FILTER_COMMAND_PRINTER_H
#define PATH_SVG_FILTER_COMMAND_PRINTER_H

#include <iostream>

#include "path/svg/isvgpath.h"

namespace rvg {
    namespace path {
        namespace svg {
            namespace filter {

class CommandPrinter final: public ISVGPath<CommandPrinter> {
private:
    std::ostream &m_out;
    int m_prev;
    char m_sep;

    template <typename ...DATA>
    void print_cmd(int cmd, DATA ...data) {
        if (m_prev != cmd) {
            if (m_prev != ' ') m_out << m_sep;
            m_out << static_cast<char>(cmd);
            m_prev = cmd;
        }
        print_data(data...);
    }

    void print_data(void) { }

    template <typename ...REST>
    void print_data(float f, REST... rest) {
        m_out << m_sep << f;
        print_data(rest...);
    }

public:
    CommandPrinter(std::ostream &out, char sep):
    m_out(out), m_prev(' '), m_sep(sep) { ; }

private:
    friend ISVGPath<CommandPrinter>;

    void do_move_to_abs(float x0, float y0) {
        print_cmd('M', x0, y0);
    }

    void do_move_to_rel(float x0, float y0) {
        print_cmd('m', x0, y0);
    }

    void do_close_path(void) {
        print_cmd('Z');
    }

    void do_line_to_abs(float x1, float y1) {
        print_cmd('L', x1, y1);
    }

    void do_line_to_rel(float x1, float y1) {
        print_cmd('l', x1, y1);
    }

    void do_hline_to_abs(float x1) {
        print_cmd('H', x1);
    }

    void do_hline_to_rel(float x1) {
        print_cmd('h', x1);
    }

    void do_vline_to_abs(float y1) {
        print_cmd('V', y1);
    }

    void do_vline_to_rel(float y1) {
        print_cmd('v', y1);
    }

    void do_quad_to_abs(float x1, float y1, float x2, float y2) {
        print_cmd('Q', x1, y1, x2, y2);
    }

    void do_quad_to_rel(float x1, float y1, float x2, float y2) {
        print_cmd('q', x1, y1, x2, y2);
    }

    void do_squad_to_abs(float x2, float y2) {
        print_cmd('T', x2, y2);
    }

    void do_squad_to_rel(float x2, float y2) {
        print_cmd('t', x2, y2);
    }

    void do_rquad_to_abs(float x1, float y1, float w1, float x2, float y2) {
        print_cmd('R', x1, y1, w1, x2, y2);
    }

    void do_rquad_to_rel(float x1, float y1, float w1, float x2, float y2) {
        print_cmd('r', x1, y1, w1, x2, y2);
    }

    void do_svgarc_to_abs(float rx, float ry, float a, float fa, float fs,
    float x2, float y2) {
        print_cmd('A', rx, ry, a, fa, fs, x2, y2);
    }

    void do_svgarc_to_rel(float rx, float ry, float a, float fa, float fs,
    float x2, float y2) {
        print_cmd('a', rx, ry, a, fa, fs, x2, y2);
    }

    void do_cubic_to_abs(float x1, float y1, float x2, float y2,
    float x3, float y3) {
        print_cmd('C', x1, y1, x2, y2, x3, y3);
    }

    void do_cubic_to_rel(float x1, float y1, float x2, float y2,
    float x3, float y3) {
        print_cmd('c', x1, y1, x2, y2, x3, y3);
    }

    void do_scubic_to_abs(float x2, float y2, float x3, float y3) {
        print_cmd('C', x2, y2, x3, y3);
    }

    void do_scubic_to_rel(float x2, float y2, float x3, float y3) {
        print_cmd('c', x2, y2, x3, y3);
    }
};

inline CommandPrinter make_command_printer(std::ostream &out, char sep) {
    return CommandPrinter{out, sep};
}

} } } } // namespace rvg::path::svg::filter

#endif
