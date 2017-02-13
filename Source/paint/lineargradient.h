#ifndef RVG_PAINT_LINEAR_GRADIENT_H
#define RVG_PAINT_LINEAR_GRADIENT_H

#include "paint/ramp.h"

namespace rvg {
    namespace paint {

class LinearGradient {
private:
    RampPtr m_ramp_ptr;
    float m_x1, m_y1, m_x2, m_y2;
public:
    LinearGradient(const RampPtr &ramp_ptr, float x1, float y1, float x2,
        float y2): m_ramp_ptr(ramp_ptr), m_x1(x1), m_y1(y1), m_x2(x2),
        m_y2(y2) { }
    const RampPtr ramp_ptr(void) const { return m_ramp_ptr; }
    const Ramp &ramp(void) const { return *m_ramp_ptr; }
    float x1(void) const { return m_x1; }
    float y1(void) const { return m_y1; }
    float x2(void) const { return m_x2; }
    float y2(void) const { return m_y2; }
};

using LinearGradientPtr = std::shared_ptr<LinearGradient>;

} } // namespace rvg::paint

#endif
