inline Projectivity Projectivity::do_scaled(float sx, float sy) const {
    return Scaling{sx, sy} * (*this);
}

inline Projectivity Projectivity::do_rotated(float cos, float sin) const {
    return Rotation{cos,sin} * (*this);
}

inline Projectivity Projectivity::do_translated(float tx, float ty) const {
    return Translation{tx, ty} * (*this);
}

inline rvg::point::RP2Tuple Projectivity::do_apply(float x, float y, float w) const {
    float rx = m_m[0][0] * x + m_m[0][1] * y + m_m[0][2] * w;
    float ry = m_m[1][0] * x + m_m[1][1] * y + m_m[1][2] * w;
    float rw = m_m[2][0] * x + m_m[2][1] * y + m_m[2][2] * w;
    return rvg::point::RP2Tuple{rx, ry, rw};
}

inline rvg::point::RP2 Projectivity::do_apply(const rvg::point::RP2 &p) const {
    float rx = m_m[0][0] * p[0] + m_m[0][1] * p[1] + m_m[0][2] * p[2];
    float ry = m_m[1][0] * p[0] + m_m[1][1] * p[1] + m_m[1][2] * p[2];
    float rw = m_m[2][0] * p[0] + m_m[2][1] * p[1] + m_m[2][2] * p[2];
    return rvg::point::RP2{rx, ry, rw};
}

inline rvg::point::RP2Tuple Projectivity::do_apply(float x, float y) const {
    float rx = m_m[0][0] * x + m_m[0][1] * y + m_m[0][2];
    float ry = m_m[1][0] * x + m_m[1][1] * y + m_m[1][2];
    float rw = m_m[2][0] * x + m_m[2][1] * y + m_m[2][2];
    return rvg::point::RP2Tuple{rx, ry, rw};
}

inline rvg::point::RP2 Projectivity::do_apply(const rvg::point::R2 &e) const {
    float rx = m_m[0][0] * e[0] + m_m[0][1] * e[1] + m_m[0][2];
    float ry = m_m[1][0] * e[0] + m_m[1][1] * e[1] + m_m[1][2];
    float rw = m_m[2][0] * e[0] + m_m[2][1] * e[1] + m_m[2][2];
    return rvg::point::RP2{rx, ry, rw};
}

inline Projectivity Projectivity::do_transformed(const Projectivity &o) const {
#ifdef XFORM_DEBUG
    std::cerr << "Projectivity.transformed(const Projectivity&)\n";
#endif
    return Projectivity(
        m_m[0][0]*o.m_m[0][0] + m_m[1][0]*o.m_m[0][1] + m_m[2][0]*o.m_m[0][2],
        m_m[0][1]*o.m_m[0][0] + m_m[1][1]*o.m_m[0][1] + m_m[2][1]*o.m_m[0][2],
        m_m[0][2]*o.m_m[0][0] + m_m[1][2]*o.m_m[0][1] + m_m[2][2]*o.m_m[0][2],
        m_m[0][0]*o.m_m[1][0] + m_m[1][0]*o.m_m[1][1] + m_m[2][0]*o.m_m[1][2],
        m_m[0][1]*o.m_m[1][0] + m_m[1][1]*o.m_m[1][1] + m_m[2][1]*o.m_m[1][2],
        m_m[0][2]*o.m_m[1][0] + m_m[1][2]*o.m_m[1][1] + m_m[2][2]*o.m_m[1][2],
        m_m[0][0]*o.m_m[2][0] + m_m[1][0]*o.m_m[2][1] + m_m[2][0]*o.m_m[2][2],
        m_m[0][1]*o.m_m[2][0] + m_m[1][1]*o.m_m[2][1] + m_m[2][1]*o.m_m[2][2],
        m_m[0][2]*o.m_m[2][0] + m_m[1][2]*o.m_m[2][1] + m_m[2][2]*o.m_m[2][2]
    );
}

inline bool Projectivity::do_is_equal(const Projectivity &o) const {
    return m_m == o.m_m;
}

inline bool Projectivity::do_is_almost_equal(const Projectivity &o) const {
    return rvg::math::is_almost_equal(m_m[0][0], o.m_m[0][0]) &&
           rvg::math::is_almost_equal(m_m[0][1], o.m_m[0][1]) &&
           rvg::math::is_almost_equal(m_m[0][2], o.m_m[0][2]) &&
           rvg::math::is_almost_equal(m_m[1][0], o.m_m[1][0]) &&
           rvg::math::is_almost_equal(m_m[1][1], o.m_m[1][1]) &&
           rvg::math::is_almost_equal(m_m[1][2], o.m_m[1][2]) &&
           rvg::math::is_almost_equal(m_m[2][0], o.m_m[2][0]) &&
           rvg::math::is_almost_equal(m_m[2][1], o.m_m[2][1]) &&
           rvg::math::is_almost_equal(m_m[2][2], o.m_m[2][2]);
}

inline Projectivity Projectivity::do_adjugate(void) const {
    return Projectivity{
       -m_m[1][2]*m_m[2][1] + m_m[1][1]*m_m[2][2],
        m_m[0][2]*m_m[2][1] - m_m[0][1]*m_m[2][2],
       -m_m[0][2]*m_m[1][1] + m_m[0][1]*m_m[1][2],
        m_m[1][2]*m_m[2][0] - m_m[1][0]*m_m[2][2],
       -m_m[0][2]*m_m[2][0] + m_m[0][0]*m_m[2][2],
        m_m[0][2]*m_m[1][0] - m_m[0][0]*m_m[1][2],
       -m_m[1][1]*m_m[2][0] + m_m[1][0]*m_m[2][1],
        m_m[0][1]*m_m[2][0] - m_m[0][0]*m_m[2][1],
       -m_m[0][1]*m_m[1][0] + m_m[0][0]*m_m[1][1]
    };
}

inline Projectivity Projectivity::do_inverse(void) const {
    float s = 1.f/do_det();
    return Projectivity{
       (-m_m[1][2]*m_m[2][1] + m_m[1][1]*m_m[2][2])*s,
       ( m_m[0][2]*m_m[2][1] - m_m[0][1]*m_m[2][2])*s,
       (-m_m[0][2]*m_m[1][1] + m_m[0][1]*m_m[1][2])*s,
       ( m_m[1][2]*m_m[2][0] - m_m[1][0]*m_m[2][2])*s,
       (-m_m[0][2]*m_m[2][0] + m_m[0][0]*m_m[2][2])*s,
       ( m_m[0][2]*m_m[1][0] - m_m[0][0]*m_m[1][2])*s,
       (-m_m[1][1]*m_m[2][0] + m_m[1][0]*m_m[2][1])*s,
       ( m_m[0][1]*m_m[2][0] - m_m[0][0]*m_m[2][1])*s,
       (-m_m[0][1]*m_m[1][0] + m_m[0][0]*m_m[1][1])*s
    };
}

inline float Projectivity::do_det(void) const {
    return -m_m[0][2]*m_m[1][1]*m_m[2][0] + m_m[0][1]*m_m[1][2]*m_m[2][0] +
        m_m[0][2]*m_m[1][0]*m_m[2][1] - m_m[0][0]*m_m[1][2]*m_m[2][1] -
        m_m[0][1]*m_m[1][0]*m_m[2][2] + m_m[0][0]*m_m[1][1]*m_m[2][2];
}

inline bool Projectivity::do_is_identity(void) const {
    return m_m[0][0] == 1.f && m_m[0][1] == 0.f && m_m[0][2] == 0.f &&
           m_m[1][0] == 1.f && m_m[1][1] == 1.f && m_m[1][2] == 0.f &&
           m_m[2][0] == 0.f && m_m[2][1] == 0.f && m_m[2][2] == 1.f;
}

inline Projectivity Projectivity::do_transpose(void) const {
    return Projectivity{
        m_m[0][0], m_m[1][0], m_m[2][0],
        m_m[0][1], m_m[1][1], m_m[2][1],
        m_m[0][2], m_m[1][2], m_m[2][2]
    };
}

inline const std::array<float,3> &Projectivity::operator[](int i) const {
    return m_m[i];
}
