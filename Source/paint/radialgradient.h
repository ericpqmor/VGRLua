#ifndef RVG_PAINT_RADIAL_GRADIENT_H
#define RVG_PAINT_RADIAL_GRADIENT_H

#include "paint/ramp.h"

namespace rvg {
    namespace paint {

class RadialGradient {
private:
    RampPtr m_ramp_ptr;
    float m_cx, m_cy, m_fx, m_fy, m_r;
public:
    RadialGradient(const RampPtr &ramp_ptr, float cx, float cy, float fx,
        float fy, float r): m_ramp_ptr(ramp_ptr), m_cx(cx), m_cy(cy), m_fx(fx),
        m_fy(fy), m_r(r) { }
    const RampPtr ramp_ptr(void) const { return m_ramp_ptr; }
    const Ramp &ramp(void) const { return *m_ramp_ptr; }
    float cx(void) const { return m_cx; }
    float cy(void) const { return m_cy; }
    float fx(void) const { return m_fx; }
    float fy(void) const { return m_fy; }
    float r(void) const { return m_r; }
};

using RadialGradientPtr = std::shared_ptr<RadialGradient>;

} } // namespace rvg::paint

#endif
