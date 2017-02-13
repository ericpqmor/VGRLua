inline Xform identity(void) {
    return static_cast<Affinity>(Identity());
}

inline Xform rotation(float deg) {
    return static_cast<Affinity>(Rotation(deg));
}

inline Xform rotation(float deg, float cx, float cy) {
    return static_cast<Affinity>(
        Translation(-cx, -cy).rotated(deg).translated(cx, cy));
}

inline Xform translation(float tx, float ty) {
    return static_cast<Affinity>(Translation(tx, ty));
}

inline Xform scaling(float sx, float sy, float cx, float cy) {
    return static_cast<Affinity>(
        Translation(-cx, -cy).scaled(sx, sy).translated(cx, cy));
}

inline Xform scaling(float s, float cx, float cy) {
    return static_cast<Affinity>(
        Translation(-cx, -cy).scaled(s).translated(cx, cy));
}

inline Xform scaling(float sx, float sy) {
    return static_cast<Affinity>(Scaling(sx, sy));
}

inline Xform scaling(float s) {
    return static_cast<Affinity>(Scaling(s, s));
}

inline Xform linear(float a, float b, float c, float d) {
    return static_cast<Affinity>(Linear(a, b, c, d));
}

inline Xform affinity(float a, float b, float tx, float c, float d, float ty) {
    return Affinity(a, b, tx, c, d, ty);
}
