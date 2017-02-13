#ifndef RVG_PATH_SVG_ISVGPATH_H
#define RVG_PATH_SVG_ISVGPATH_H

namespace rvg {
    namespace path {
        namespace svg {

template <typename DERIVED>
class ISVGPath {
protected:
    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }
    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }
public:
    void move_to_abs(float x0, float y0) {
        return derived().do_move_to_abs(x0, y0);
    }

    void move_to_rel(float x0, float y0) {
        return derived().do_move_to_rel(x0, y0);
    }

    void close_path(void) {
        return derived().do_close_path();
    }

    void line_to_abs(float x1, float y1) {
        return derived().do_line_to_abs(x1, y1);
    }

    void line_to_rel(float x1, float y1) {
        return derived().do_line_to_rel(x1, y1);
    }

    void hline_to_abs(float x1) {
        return derived().do_hline_to_abs(x1);
    }

    void hline_to_rel(float x1) {
        return derived().do_hline_to_rel(x1);
    }

    void vline_to_abs(float y1) {
        return derived().do_vline_to_abs(y1);
    }

    void vline_to_rel(float y1) {
        return derived().do_vline_to_rel(y1);
    }

    void quad_to_abs(float x1, float y1, float x2, float y2) {
        return derived().do_quad_to_abs(x1, y1, x2, y2);
    }

    void quad_to_rel(float x1, float y1, float x2, float y2) {
        return derived().do_quad_to_rel(x1, y1, x2, y2);
    }

    void squad_to_abs(float x2, float y2) {
        return derived().do_squad_to_abs(x2, y2);
    }

    void squad_to_rel(float x2, float y2) {
        return derived().do_squad_to_rel(x2, y2);
    }

    void rquad_to_abs(float x1, float y1, float w1, float x2, float y2) {
        return derived().do_rquad_to_abs(x1, y1, w1, x2, y2);
    }

    void rquad_to_rel(float x1, float y1, float w1, float x2, float y2) {
        return derived().do_rquad_to_rel(x1, y1, w1, x2, y2);
    }

    void svgarc_to_abs(float rx, float ry, float a, float fa, float fs,
        float x2, float y2) {
        return derived().do_svgarc_to_abs(rx, ry, a, fa, fs, x2, y2);
    }

    void svgarc_to_rel(float rx, float ry, float a, float fa, float fs,
        float x2, float y2) {
        return derived().do_svgarc_to_rel(rx, ry, a, fa, fs, x2, y2);
    }

    void cubic_to_abs(float x1, float y1, float x2, float y2,
        float x3, float y3) {
        return derived().do_cubic_to_abs(x1, y1, x2, y2, x3, y3);
    }

    void cubic_to_rel(float x1, float y1, float x2, float y2,
        float x3, float y3) {
        return derived().do_cubic_to_rel(x1, y1, x2, y2, x3, y3);
    }

    void scubic_to_abs(float x2, float y2, float x3, float y3) {
        return derived().do_scubic_to_abs(x2, y2, x3, y3);
    }

    void scubic_to_rel(float x2, float y2, float x3, float y3) {
        return derived().do_scubic_to_rel(x2, y2, x3, y3);
    }

};

} } } // namespace rvg::path::svg

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_isvgpath = std::integral_constant<
    bool,
    is_crtp_of<
        rvg::path::svg::ISVGPath,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // namespace rvg::meta

#endif
