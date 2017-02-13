#ifndef RVG_STROKE_ISTROKABLE_H
#define RVG_STROKE_ISTROKABLE_H

#include "stroke/join.h"
#include "stroke/cap.h"
#include "stroke/method.h"
#include "stroke/dasharray.h"

namespace rvg {
    namespace stroke {

// This is used for static polymorphism
// Modified stroke styles can be created by a variety of methods.
// Using the same interface, we can create stroked shapes with modified styles.
template <typename DERIVED, typename STYLE>
class IStrokable {
    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }
public:
    DERIVED dashed(const DashArray &dash_array) const {
        return derived().do_dashed(dash_array);
    }

    DERIVED dashed(const DashArray &dash_array,
        float initial_phase, bool phase_reset) const {
        return derived().do_dashed(dash_array, initial_phase, phase_reset);
    }

    DERIVED stroked(float width) const {
        return derived().do_stroked(width);
    }

    DERIVED stroked(const STYLE &style) const {
        return derived().do_stroked(style);
    }

    DERIVED capped(Cap cap) const {
        return derived().do_capped(cap);
    }

    DERIVED joined(Join join) const {
        return derived().do_joined(join);
    }

    DERIVED joined(Join join, float miter_limit) const {
        return derived().do_joined(join, miter_limit);
    }

    DERIVED by(Method method) const {
        return derived().do_by(method);
    }
};

} } // namespace rvg::stroke


#endif
