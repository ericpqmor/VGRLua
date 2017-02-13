#ifndef RVG_SHAPE_POLYGON_H
#define RVG_SHAPE_POLYGON_H

#include <memory>
#include <vector>
#include <type_traits>
#include <iterator>

#include "path/path.h"
#include "path/filter/close-path.h"
#include "path/filter/bracket-lengths.h"
#include "xform/xform.h"
#include "meta/meta.h"

template <typename IT>
using is_float_iterator = std::integral_constant<bool,
    std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<IT>::iterator_category>::value &&
    std::is_same<typename std::iterator_traits<IT>::value_type, float>::value>;

namespace rvg {
    namespace shape {

class Polygon {

std::vector<float> m_coordinates;

public:
    Polygon(const std::initializer_list<float> &coordinates):
        m_coordinates(coordinates) { ; }

    template <typename SIT>
    Polygon(SIT &begin, const SIT &end, typename std::enable_if<
		is_float_iterator<SIT>::value>::type * = nullptr):
        m_coordinates(begin, end) { ; }

    const std::vector<float> coordinates(void) const { return m_coordinates; }

    path::PathPtr as_path_ptr(const xform::Xform &post_xf) const {
        (void) post_xf;
        auto p = std::make_shared<path::Path>();
        auto f = path::filter::make_close_path(
            path::filter::make_bracket_lengths(*p));
        auto n = m_coordinates.size()/2;
        if (n >= 1) {
            f.begin_closed_contour(0, m_coordinates[0], m_coordinates[1]);
            for (unsigned i = 0; i < n-1; ++i) {
                f.linear_segment(m_coordinates[2*i], m_coordinates[2*i+1],
                    m_coordinates[2*i+2], m_coordinates[2*i+3]);
            }
            f.end_closed_contour(m_coordinates[2*(n-1)],
                m_coordinates[2*(n-1)+1], 0);
        }
        return p;
    }
};

using PolygonPtr = std::shared_ptr<Polygon>;

} } // namespace rvg::shape

#endif
