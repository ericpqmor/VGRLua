inline rvg::point::RP2Tuple Translation::do_apply(float x, float y, float w) const {
    return rvg::point::RP2Tuple{x + m_tx*w, y+m_ty*w, w};
}

inline rvg::point::RP2 Translation::do_apply(const rvg::point::RP2 &p) const {
    return rvg::point::RP2{p[0] + m_tx*p[2], p[1]+m_ty*p[2], p[2]};
}

inline rvg::point::R2Tuple Translation::do_apply(float x, float y) const {
    return rvg::point::R2Tuple{x + m_tx, y + m_ty};
}

inline rvg::point::R2 Translation::do_apply(const rvg::point::R2 &e) const {
    return rvg::point::R2{e[0] + m_tx, e[1] + m_ty};
}

inline Translation Translation::do_transformed(const Translation &o) const {
#ifdef XFORM_DEBUG
    std::cerr << "Translation.operator*(const Translation &)\n";
#endif
    return Translation{m_tx+o.m_tx, m_ty+o.m_ty};
}

inline Affinity Translation::do_scaled(float sx, float sy) const {
    return Scaling{sx, sy} * (*this);
}

inline Affinity Translation::do_rotated(float cos, float sin) const {
    return Rotation{cos, sin} * (*this);
}

inline Translation Translation::do_translated(float tx, float ty) const {
    return do_transformed(Translation(tx, ty));
}

inline bool Translation::do_is_almost_equal(const Translation &o) const {
    return rvg::math::is_almost_equal(m_tx, o.m_tx) &&
           rvg::math::is_almost_equal(m_ty, o.m_ty);
}

inline bool Translation::do_is_equal(const Translation &o) const {
    return m_tx == o.m_tx && m_ty == o.m_ty;
}

inline Translation Translation::do_adjugate(void) const {
    return Translation{-m_tx, -m_ty};
}

inline Translation Translation::do_inverse(void) const {
    return Translation{-m_tx, -m_ty};
}

inline bool Translation::do_is_identity(void) const {
    return m_tx == 0.f && m_ty == 0.f;
}

inline Projectivity Translation::do_transpose(void) const {
    return Projectivity{1, 0, 0, 0, 1, 0, m_tx, m_ty, 1};
}

inline float Translation::do_det(void) const {
    return 1.f;
}

inline float Translation::tx(void) const {
    return m_tx;
}

inline float Translation::ty(void) const {
    return m_ty;
}
