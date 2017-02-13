#ifndef RVG_XFORM_SVD_H
#define RVG_XFORM_SVD_H

#include "xform/xform.h"

namespace rvg {
    namespace xform {

// analytic singular value decomposition of 2D matrix
// if you only care about U and S
void svd(const Linear &A, Rotation &U, Scaling &S);

// analytic singular value decomposition of 2D matrix
// if you care about U, S and V
void svd(const Linear &A, Rotation &U, Scaling &S, Linear &V);

}} // namespace rvg::xform

#endif
