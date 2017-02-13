inline rvg::point::RP2Tuple Identity::do_apply(float x, float y, float w) const {
    return rvg::point::RP2Tuple{x, y, w};
}

inline rvg::point::R2Tuple Identity::do_apply(float x, float y) const {
    return rvg::point::R2Tuple{x, y};
}

inline rvg::point::RP2 Identity::do_apply(const rvg::point::RP2 &p) const {
    return p;
}

inline rvg::point::R2 Identity::do_apply(const rvg::point::R2 &e) const {
    return e;
}

inline Identity Identity::do_transformed(const Identity &) const {
#ifdef XFORM_DEBUG
    std::cerr << "Identity.operator*(const Identity &)\n";
#endif
    return Identity{};
}

inline Rotation Identity::do_rotated(float cos, float sin) const {
    return Rotation{cos, sin};
}

inline Translation Identity::do_translated(float tx, float ty) const {
    return Translation{tx, ty};
}

inline Scaling Identity::do_scaled(float sx, float sy) const {
    return Scaling{sx, sy};
}

inline bool Identity::do_is_equal(const Identity &) const {
    return true;
}

inline bool Identity::do_is_almost_equal(const Identity &) const {
    return true;
}

inline Identity Identity::do_adjugate(void) const {
    return Identity{};
}

inline bool Identity::do_is_identity(void) const {
    return true;
}

inline Identity Identity::do_inverse(void) const {
    return Identity{};
}

inline Identity Identity::do_transpose(void) const {
    return Identity{};
}

inline float Identity::do_det(void) const {
    return 1.f;
}
