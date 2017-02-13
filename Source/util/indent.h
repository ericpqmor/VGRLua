#ifndef RVG_UTIL_INDENT_H
#define RVG_UTIL_INDENT_H

namespace rvg {
    namespace util {

class Indent {
    int m_n;
    const char *m_v;
public:
    explicit Indent(int n = 0, const char *v = "  "): m_n(n), m_v(v) { ; }

    Indent operator++(int) { Indent x = *this; ++m_n; return x; }

    Indent operator--(int) { Indent x = *this; --m_n; return x; }

    Indent &operator--() { --m_n; return *this; }

    Indent &operator++() { ++m_n; return *this; }

    void print(std::ostream &out) const {
        out << '\n';
        for (int i = 0; i < m_n; i++) out << "  ";
    }
};

inline std::ostream &operator<<(std::ostream &out, const Indent &id) {
    id.print(out);
    return out;
}

} } // namespace rvg::util

#endif
