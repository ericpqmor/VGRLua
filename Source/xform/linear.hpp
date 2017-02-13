inline Linear Linear::do_scaled(float sx, float sy) const {
    return Scaling{sx, sy} * (*this);
}

inline Linear Linear::do_rotated(float cos, float sin) const {
    return Rotation{cos, sin} * (*this);
}

inline Affinity Linear::do_translated(float tx, float ty) const {
    return Translation{tx, ty} * (*this);
}

inline rvg::point::RP2Tuple Linear::do_apply(float x, float y, float w) const {
    float rx = m_m[0][0] * x + m_m[0][1] * y;
    float ry = m_m[1][0] * x + m_m[1][1] * y;
    return rvg::point::RP2Tuple{rx, ry, w};
}

inline rvg::point::RP2 Linear::do_apply(const rvg::point::RP2 &p) const {
    float rx = m_m[0][0] * p[0] + m_m[0][1] * p[1];
    float ry = m_m[1][0] * p[0] + m_m[1][1] * p[1];
    return rvg::point::RP2{rx, ry, p[2]};
}

inline rvg::point::R2Tuple Linear::do_apply(float x, float y) const {
    float rx = m_m[0][0] * x + m_m[0][1] * y;
    float ry = m_m[1][0] * x + m_m[1][1] * y;
    return rvg::point::R2Tuple{rx, ry};
}

inline rvg::point::R2 Linear::do_apply(const rvg::point::R2 &e) const {
    float rx = m_m[0][0] * e[0] + m_m[0][1] * e[1];
    float ry = m_m[1][0] * e[0] + m_m[1][1] * e[1];
    return rvg::point::R2{rx, ry};
}

inline Linear Linear::do_transformed(const Linear &o) const {
#ifdef XFORM_DEBUG
std::cerr << "Linear.operator*(Linear)\n";
#endif
    return Linear{m_m[0][0]*o.m_m[0][0] + m_m[1][0]*o.m_m[0][1],
        m_m[0][1]*o.m_m[0][0] + m_m[1][1]*o.m_m[0][1],
        m_m[0][0]*o.m_m[1][0] + m_m[1][0]*o.m_m[1][1],
        m_m[0][1]*o.m_m[1][0] + m_m[1][1]*o.m_m[1][1]};
}

inline bool Linear::do_is_almost_equal(const Linear &o) const {
    return rvg::math::is_almost_equal(m_m[0][0], o.m_m[0][0]) &&
           rvg::math::is_almost_equal(m_m[0][1], o.m_m[0][1]) &&
           rvg::math::is_almost_equal(m_m[1][0], o.m_m[1][0]) &&
           rvg::math::is_almost_equal(m_m[1][1], o.m_m[1][1]);
}

inline bool Linear::do_is_equal(const Linear &o) const {
    return m_m == o.m_m;
}

inline bool Linear::do_is_identity(void) const {
    return m_m[0][0] == 1.f && m_m[0][1] == 0.f &&
           m_m[1][0] == 0.f && m_m[1][1] == 1.f;
}

inline Projectivity Linear::do_adjugate(void) const {
    return Projectivity{
        m_m[1][1], -m_m[0][1], 0,
       -m_m[1][0],  m_m[0][0], 0,
                0,          0, do_det()
    };
}

inline Linear Linear::do_inverse(void) const {
    float s = 1.f/do_det();
    return Linear{
        m_m[1][1]*s, -m_m[0][1]*s,
       -m_m[1][0]*s,  m_m[0][0]*s
    };
}

inline Linear Linear::do_transpose(void) const {
    return Linear{m_m[0][0], m_m[1][0], m_m[0][1], m_m[1][1]};
}

inline float Linear::do_det(void) const {
    return m_m[0][0]*m_m[1][1] - m_m[0][1]*m_m[1][0];
}

inline const std::array<float,2> &Linear::operator[](int i) const {
    return m_m[i];
}
