#ifndef RVG_PAINT_RAMP_H
#define RVG_PAINT_RAMP_H

#include <vector>
#include <type_traits>
#include <iterator>

#include "color/color.h"
#include "meta/meta.h"
#include "paint/spread.h"

namespace rvg {
    namespace paint {

class Stop {
    float m_offset;
    rvg::color::RGBA8 m_color;
public:
    Stop(void): m_offset(0.f), m_color(rvg::color::RGBA8()) { ; }
    Stop(float offset, rvg::color::RGBA8 color):
        m_offset(offset), m_color(color) { ; }
    float offset(void) const { return m_offset; }
    rvg::color::RGBA8 color(void) const { return m_color; }
};

template <typename IT>
using is_stop_iterator = std::integral_constant<bool,
    std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IT>::iterator_category>::value &&
    std::is_same<typename std::iterator_traits<IT>::value_type, Stop>::value>;

class Ramp {
    Spread m_spread;
    std::vector<Stop> m_stops;
public:
	// construct from initializer list
    Ramp(Spread spread, const std::initializer_list<Stop> &stops):
        m_spread(spread), m_stops(stops) { };
	// construct from stop iterator
    template <typename SIT>
    Ramp(Spread spread, SIT &begin, const SIT &end, typename std::enable_if<
		is_stop_iterator<SIT>::value>::type * = nullptr):
		m_spread(spread), m_stops(begin, end) {
	}
    Spread spread(void) const { return m_spread; }
    const std::vector<Stop> &stops(void) const { return m_stops; }
};

using RampPtr = std::shared_ptr<Ramp>;

} } // namespace rvg::paint

#endif
