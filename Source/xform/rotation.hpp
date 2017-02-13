// apply transformation to vector
inline rvg::point::RP2Tuple Rotation::do_apply(float x, float y, float w) const {
    return rvg::point::RP2Tuple{m_cos * x - m_sin * y, m_sin * x + m_cos * y, w};
}

inline rvg::point::R2Tuple Rotation::do_apply(float x, float y) const {
    return rvg::point::R2Tuple{m_cos * x - m_sin * y, m_sin * x + m_cos * y};
}

inline rvg::point::RP2 Rotation::do_apply(const rvg::point::RP2 &p) const {
    return rvg::point::RP2{m_cos * p[0] - m_sin * p[1],
        m_sin * p[0] + m_cos * p[1], p[2]};
}

inline rvg::point::R2 Rotation::do_apply(const rvg::point::R2 &e) const {
    return rvg::point::R2{m_cos * e[0] - m_sin * e[1],
        m_sin * e[0] + m_cos * e[1]};
}

inline Rotation Rotation::do_transformed(const Rotation &o) const {
#ifdef XFORM_DEBUG
    std::cerr << "Rotation.operator*(const Rotation &)\n";
#endif
    return Rotation(m_cos*o.m_cos - m_sin*o.m_sin, m_sin*o.m_cos + m_cos*o.m_sin);
}

inline Rotation Rotation::do_rotated(float cos, float sin) const {
    return do_transformed(Rotation{cos, sin});
}

inline Linear Rotation::do_scaled(float sx, float sy) const {
    return Scaling{sx, sy} * (*this);
}

inline Affinity Rotation::do_translated(float tx, float ty) const {
    return Translation{tx, ty} * (*this);
}

inline bool Rotation::do_is_equal(const Rotation &o) const {
    return m_cos == o.m_cos && m_sin == o.m_sin;
}

inline bool Rotation::do_is_almost_equal(const Rotation &o) const {
    return rvg::math::is_almost_equal(m_cos, o.m_cos) &&
           rvg::math::is_almost_equal(m_sin, o.m_sin);
}

inline Projectivity Rotation::do_adjugate(void) const {
    return Projectivity{
        m_cos, m_sin, 0,
       -m_sin, m_cos, 0,
            0,     0, do_det()
    };
}

inline bool Rotation::do_is_identity(void) const {
    return m_cos == 1.f && m_sin == 0.f;
}

inline Rotation Rotation::do_inverse(void) const {
    float s = 1.f/do_det();
    return Rotation{m_cos*s, -m_sin*s};
}

inline Rotation Rotation::do_transpose(void) const {
    return Rotation{m_cos, -m_sin};
}

inline float Rotation::do_det(void) const {
    return m_cos*m_cos + m_sin*m_sin;
}

inline float Rotation::cos(void) const {
    return m_cos;
}

inline float Rotation::sin(void) const {
    return m_sin;
}
