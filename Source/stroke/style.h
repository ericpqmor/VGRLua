#ifndef RVG_STROKE_STYLE_H
#define RVG_STROKE_STYLE_H

#include <iosfwd>

#include "stroke/istrokable.h"

namespace rvg {
    namespace stroke {

class Style final: public IStrokable<Style, Style> {
public:
    using Base = IStrokable<Style, Style>;

    static constexpr Method default_method = Method::driver;
    static constexpr Cap default_cap = Cap::butt;
    static constexpr Join default_join = Join::miter;
    static constexpr float default_width = 0.f;
    static constexpr float default_miter_limit = 4.f;
    static constexpr bool default_phase_reset = false;
    static constexpr float default_initial_phase = 0.f;
    static const DashArrayPtr default_dash_array_ptr;

private:
    Cap m_cap;
    Join m_join;
    Method m_method;
    bool m_phase_reset;
    float m_width, m_miter_limit, m_initial_phase;
    DashArrayPtr m_dash_array_ptr;

public:

    Style(void):
        m_cap(default_cap),
        m_join(default_join),
        m_method(default_method),
        m_phase_reset(default_phase_reset), m_width(default_width),
        m_miter_limit(default_miter_limit),
        m_initial_phase(default_initial_phase),
        m_dash_array_ptr(default_dash_array_ptr) { ; }

    const DashArrayPtr &dash_array_ptr(void) const
    { return m_dash_array_ptr; }

    const DashArray &dash_array(void) const
    { return *m_dash_array_ptr; }

    float width(void) const
    { return m_width; }

    Join join(void) const
    { return m_join; }

    Cap cap(void) const
    { return m_cap; }

    Method method(void) const
    { return m_method; }

    float miter_limit(void) const
    { return m_miter_limit; }

    float initial_phase(void) const
    { return m_initial_phase; }

    bool phase_reset(void) const
    { return m_phase_reset; }

private: // IStrokable interface

    friend Base;

    Style do_dashed(const DashArray &dash_array) const {
        Style copy(*this);
        copy.m_dash_array_ptr = std::make_shared<DashArray>(dash_array);
        return copy;
    }

    Style do_dashed(const DashArray &dash_array,
        float initial_phase, bool phase_reset) const {
        Style copy(*this);
        copy.m_dash_array_ptr = std::make_shared<DashArray>(dash_array);
        copy.m_initial_phase = initial_phase;
        copy.m_phase_reset = phase_reset;
        return copy;
    }

    Style do_stroked(float width) const {
        Style copy(*this);
        copy.m_width = width;
        return copy;
    }

    Style do_stroked(const Style &style) const {
        return style;
    }

    Style do_capped(Cap cap) const {
        Style copy(*this);
        copy.m_cap = cap;
        return copy;
    }

    Style do_joined(Join join) const {
        Style copy(*this);
        copy.m_join = join;
        return copy;
    }

    Style do_joined(Join join, float miter_limit) const {
        Style copy(*this);
        copy.m_join = join;
        copy.m_miter_limit = miter_limit;
        return copy;
    }

    Style do_by(Method method) const {
        Style copy(*this);
        copy.m_method = method;
        return copy;
    }
};

using StylePtr = std::shared_ptr<Style>;

std::ostream &operator<<(std::ostream &out, const Style &style);

} } // namespace rvg::stroke

#endif
