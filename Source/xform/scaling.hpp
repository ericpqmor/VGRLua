// apply transformation to vector
inline rvg::point::RP2Tuple Scaling::do_apply(float x, float y, float w) const {
    return rvg::point::RP2Tuple{x * m_sx, y * m_sy, w};
}

inline rvg::point::R2Tuple Scaling::do_apply(float x, float y) const {
    return rvg::point::R2Tuple{x * m_sx, y * m_sy};
}

inline rvg::point::RP2 Scaling::do_apply(const rvg::point::RP2 &p) const {
    return rvg::point::RP2{p[0] * m_sx, p[1] * m_sy, p[2]};
}

inline rvg::point::R2 Scaling::do_apply(const rvg::point::R2 &e) const {
    return rvg::point::R2{e[0] * m_sx, e[1] * m_sy};
}

inline Scaling Scaling::do_transformed(const Scaling &o) const {
#ifdef XFORM_DEBUG
    std::cerr << "Scaling.operator*(const Scaling &)\n";
#endif
    return Scaling(m_sx*o.m_sx, m_sy*o.m_sy);
}

inline Linear Scaling::do_rotated(float cos, float sin) const {
    return Rotation{cos, sin} * (*this);
}

inline Scaling Scaling::do_scaled(float sx, float sy) const {
    return Scaling(sx, sy) * (*this);
}

inline Affinity Scaling::do_translated(float tx, float ty) const {
    return Translation(tx, ty) * (*this);
}

inline bool Scaling::do_is_almost_equal(const Scaling &o) const {
    return rvg::math::is_almost_equal(m_sx, o.m_sx) &&
           rvg::math::is_almost_equal(m_sy, o.m_sy);
}

inline bool Scaling::do_is_equal(const Scaling &o) const {
    return m_sx == o.m_sx && m_sy == o.m_sy;
}

inline Projectivity Scaling::do_adjugate(void) const {
    return Projectivity{m_sy, 0, 0, 0, m_sx, 0, 0, 0, do_det()};
}

inline Scaling Scaling::do_transpose(void) const {
    return *this;
}

inline bool Scaling::do_is_identity(void) const {
    return m_sx == 1.f && m_sy == 1.f;
}

inline Scaling Scaling::do_inverse(void) const {
    return Scaling{1.f/m_sx, 1.f/m_sy};
}

inline float Scaling::do_det(void) const {
    return m_sx*m_sy;
}

inline float Scaling::sx(void) const {
    return m_sx;
}

inline float Scaling::sy(void) const {
    return m_sy;
}
