#ifndef RVG_XFORM_H
#define RVG_XFORM_H

#ifdef XFORM_DEBUG
#include <iostream>
#endif

// Our Xforms are transformations, and should not be confused
// with the matrices that represent them.
// These transformations operate on Euclidean points in R^2, on
// projective points in RP^2, and on each other.
// This means that if you multiply a transformation by a
// scalar, you are not multiplying the matrix by that
// scalar. You are creating a transformation that, when
// applied to a point, results in a scaled point.
// We define the scaling of points in the Euclydean plane.
// So the projective representation will not have all its
// coordinates multiplied by the scalar. Only the first two.
// By the same philosophy, our projective points can be
// added and multiplied by scalars.
#include "xform/ixform.h"
#include "xform/projectivity.h"
#include "xform/affinity.h"
#include "xform/linear.h"
#include "xform/translation.h"
#include "xform/rotation.h"
#include "xform/scaling.h"
#include "xform/identity.h"
#include "xform/windowviewport.h"

namespace rvg {
    namespace xform {

// This is the default transformation type used
using Xform = Affinity;

// Convenience functions
Xform identity(void);
Xform rotation(float ang);
Xform rotation(float ang, float cx, float cy);
Xform translation(float tx, float ty);
Xform scaling(float sx, float sy, float cx, float cy);
Xform scaling(float s, float cx, float cy);
Xform scaling(float sx, float sy);
Xform scaling(float s);
Xform linear(float a, float b, float c, float d);
Xform affinity(float a, float b, float tx, float c, float d, float ty);

template <typename M,
    typename = typename std::enable_if<
        rvg::meta::is_an_ixform<M>::value>::type>
float det(const M &m) {
    return m.det();
}

template <typename M,
    typename = typename std::enable_if<
        rvg::meta::is_an_ixform<M>::value>::type>
decltype(std::declval<const M&>().transpose())
transpose(const M &m) {
    return m.transpose();
}

template <typename M,
    typename = typename std::enable_if<
        rvg::meta::is_an_ixform<M>::value>::type>
decltype(std::declval<const M&>().adjugate())
adjugate(const M &m) {
    return m.adjugate();
}

#include "xform/mixed-product.hpp"
#include "xform/projectivity.hpp"
#include "xform/affinity.hpp"
#include "xform/linear.hpp"
#include "xform/translation.hpp"
#include "xform/rotation.hpp"
#include "xform/scaling.hpp"
#include "xform/identity.hpp"
#include "xform/xform.hpp"

} } // namespace rvg::xform

#endif
