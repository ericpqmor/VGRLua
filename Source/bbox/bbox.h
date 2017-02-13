#ifndef RVG_BBOX_H
#define RVG_BBOX_H

#include <limits>
#include <array>
#include <tuple>

namespace rvg {
    namespace bbox {

template <typename F>
class BBox {
    std::array<F, 4> m_corners;
    constexpr static F lo = std::numeric_limits<F>::lowest();
    constexpr static F hi = std::numeric_limits<F>::max();

public:
    using value_type = F;

    BBox(): m_corners{hi, hi, lo, lo} { ; }

    BBox(F xl, F yb, F xr, F yt): m_corners{xl, yb, xr, yt} { ; }

    std::tuple<F, F> bl(void) const {
        return std::tuple<F, F>(m_corners[0], m_corners[1]);
    }

    std::tuple<F, F> tr(void) const {
        return std::tuple<F, F>(m_corners[2], m_corners[3]);
    }

    void set_bl(float xl, float yb) {
        m_corners[0] = xl;
        m_corners[1] = yb;
    }

    void set_tr(float xr, float yt) {
        m_corners[2] = xr;
        m_corners[3] = yt;
    }

    const std::array<F, 4> &corners(void) const {
        return m_corners;
    }
};

} } // namespace rvg::bbox

#endif
