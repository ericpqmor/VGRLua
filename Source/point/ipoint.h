#ifndef RVG_POINT_IPOINT_H
#define RVG_POINT_IPOINT_H

#include "meta/meta.h"

namespace rvg {
    namespace point {

template <typename DERIVED>
class IPoint {
protected:
    DERIVED &derived(void) {
        return *static_cast<DERIVED *>(this);
    }

    const DERIVED &derived(void) const {
        return *static_cast<const DERIVED *>(this);
    }

public:
    DERIVED added(const DERIVED &o) const {
        return derived().do_added(o);
    }

    DERIVED subtracted(const DERIVED &o) const {
        return derived().do_subtracted(o);
    }

    DERIVED negated(const DERIVED &o) const {
        return derived().do_negated(o);
    }

    DERIVED scaled(float s) const {
        return derived().do_scaled(s);
    }

	float component(int i) const {
        return derived().do_component(i);
	}

	bool is_almost_equal(const DERIVED &o) const {
		return derived().do_is_almost_equal(o);
	}

	bool is_almost_ideal(void) const {
		return derived().do_is_almost_ideal();
	}

	std::ostream &print(std::ostream &out) const {
		return derived().do_print(out);
	}

	float operator[](int i) const {
		return derived().do_component(i);
	}

	DERIVED operator+(const DERIVED &o) const {
		return derived().do_added(o);
	}

	DERIVED operator-(const DERIVED &o) const {
		return derived().do_subtracted(o);
	}

	DERIVED operator-(void) const {
		return derived().do_negated();
	}

	DERIVED operator*(float s) const {
		return derived().do_scaled(s);
	}

    bool is_equal(const DERIVED &o) const {
        return derived().do_is_equal();
    }

    bool operator==(const DERIVED &o) const {
        return derived().do_is_equal(o);
    }

    bool operator!=(const DERIVED &o) const {
        return !derived().do_is_equal(o);
    }

#ifdef HAS_CPP14_AUTO_RETURN
    auto untie(void) const
#else
    template <typename D = DERIVED>
    auto untie(void) const ->
        decltype(std::declval<const D&>().do_untie())
#endif
    {
        return derived().do_untie();
    }
};

template <typename DERIVED>
std::ostream &operator<<(std::ostream &out, const IPoint<DERIVED> &p) {
    return p.print(out);
}

} } // namespace rvg::point

namespace rvg {
    namespace meta {

template <typename DERIVED>
using is_an_ipoint = std::integral_constant<
    bool,
    is_crtp_of<
        rvg::point::IPoint,
        typename remove_reference_cv<DERIVED>::type
    >::value>;

} } // rvg::meta

#endif
