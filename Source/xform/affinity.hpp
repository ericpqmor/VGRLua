inline Affinity Affinity::do_scaled(float sx, float sy) const {
    return Scaling{sx, sy} * (*this);
}

inline Affinity Affinity::do_rotated(float cos, float sin) const {
    return Rotation{cos, sin} * (*this);
}

inline Affinity Affinity::do_translated(float tx, float ty) const {
    return Translation(tx, ty) * (*this);
}

inline rvg::point::RP2Tuple Affinity::do_apply(float x, float y, float w) const {
    float rx = m_m[0][0] * x + m_m[0][1] * y + m_m[0][2] * w;
    float ry = m_m[1][0] * x + m_m[1][1] * y + m_m[1][2] * w;
    return rvg::point::RP2Tuple{rx, ry, w};
}

inline rvg::point::RP2 Affinity::do_apply(const rvg::point::RP2 &p) const {
    float rx = m_m[0][0] * p[0] + m_m[0][1] * p[1] + m_m[0][2] * p[2];
    float ry = m_m[1][0] * p[0] + m_m[1][1] * p[1] + m_m[1][2] * p[2];
    return rvg::point::RP2{rx, ry, p[2]};
}

inline rvg::point::R2Tuple Affinity::do_apply(const float x, float y) const {
    float rx = m_m[0][0] * x + m_m[0][1] * y + m_m[0][2];
    float ry = m_m[1][0] * x + m_m[1][1] * y + m_m[1][2];
    return rvg::point::R2Tuple{rx, ry};
}

inline rvg::point::R2 Affinity::do_apply(const rvg::point::R2 &e) const {
    float rx = m_m[0][0] * e[0] + m_m[0][1] * e[1] + m_m[0][2];
    float ry = m_m[1][0] * e[0] + m_m[1][1] * e[1] + m_m[1][2];
    return rvg::point::R2{rx, ry};
}

inline Affinity Affinity::do_transformed(const Affinity &o) const {
#ifdef XFORM_DEBUG
    std::cerr << "Affinity.operator*(const Affinity &)\n";
#endif
    return Affinity(
        m_m[0][0]*o.m_m[0][0] + m_m[1][0]*o.m_m[0][1],
        m_m[0][1]*o.m_m[0][0] + m_m[1][1]*o.m_m[0][1],
        m_m[0][2]*o.m_m[0][0] + m_m[1][2]*o.m_m[0][1] + o.m_m[0][2],
        m_m[0][0]*o.m_m[1][0] + m_m[1][0]*o.m_m[1][1],
        m_m[0][1]*o.m_m[1][0] + m_m[1][1]*o.m_m[1][1],
        m_m[0][2]*o.m_m[1][0] + m_m[1][2]*o.m_m[1][1] + o.m_m[1][2]
    );
}

inline bool Affinity::do_is_almost_equal(const Affinity &o) const {
    return rvg::math::is_almost_equal(m_m[0][0], o.m_m[0][0]) &&
           rvg::math::is_almost_equal(m_m[0][1], o.m_m[0][1]) &&
           rvg::math::is_almost_equal(m_m[1][0], o.m_m[1][0]) &&
           rvg::math::is_almost_equal(m_m[1][1], o.m_m[1][1]) &&
           rvg::math::is_almost_equal(m_m[0][2], o.m_m[0][2]) &&
           rvg::math::is_almost_equal(m_m[1][2], o.m_m[1][2]);
}

inline bool Affinity::do_is_identity(void) const {
    return m_m[0][0] == 1.f && m_m[0][1] == 0.f &&
           m_m[1][0] == 0.f && m_m[1][1] == 1.f &&
           m_m[0][2] == 0.f && m_m[1][2] == 0.f ;
}

inline bool Affinity::do_is_equal(const Affinity &o) const {
    return m_m == o.m_m;
}

inline Projectivity Affinity::do_adjugate(void) const {
    return Projectivity{
        m_m[1][1], -m_m[0][1], -m_m[0][2]*m_m[1][1] + m_m[0][1]*m_m[1][2],
       -m_m[1][0],  m_m[0][0],  m_m[0][2]*m_m[1][0] - m_m[0][0]*m_m[1][2],
                0,          0,  do_det()
    };
}

inline Affinity Affinity::do_inverse(void) const {
    float s = 1.f/do_det();
    return Affinity{
        m_m[1][1]*s, -m_m[0][1]*s, (-m_m[0][2]*m_m[1][1]+m_m[0][1]*m_m[1][2])*s,
       -m_m[1][0]*s,  m_m[0][0]*s, ( m_m[0][2]*m_m[1][0]-m_m[0][0]*m_m[1][2])*s
    };
}

inline Projectivity Affinity::do_transpose(void) const {
    return Projectivity{
        m_m[0][0], m_m[1][0], 0,
        m_m[0][1], m_m[1][1], 0,
        m_m[0][2], m_m[1][2], 1
    };
}

inline float Affinity::do_det(void) const {
    return m_m[0][0]*m_m[1][1] - m_m[0][1]*m_m[1][0];
}

inline const std::array<float,3> &Affinity::operator[](int i) const {
    return m_m[i];
}

