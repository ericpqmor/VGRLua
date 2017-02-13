template <typename M, typename N,
    typename = typename std::enable_if<
        rvg::meta::is_an_ixform<M>::value &&
        rvg::meta::is_an_ixform<N>::value &&
        !std::is_same<M, N>::value &&
        std::is_convertible<M,Projectivity>::value &&
        std::is_convertible<N,Projectivity>::value &&
        (!std::is_convertible<M,Affinity>::value ||
         !std::is_convertible<N,Affinity>::value)>::type>
Projectivity operator*(const M &m, const N &n) {
#ifdef XFORM_DEBUG
    std::cerr << "Projectivity operator*(static_cast<Projectivity>, static_cast<Projectivity>)\n";
#endif
    return static_cast<Projectivity>(n).transformed(static_cast<Projectivity>(m));
}

template <typename M, typename N,
    typename = typename std::enable_if<
        rvg::meta::is_an_ixform<M>::value &&
        rvg::meta::is_an_ixform<N>::value &&
        !std::is_same<M, N>::value &&
        std::is_convertible<M,Affinity>::value &&
        std::is_convertible<N,Affinity>::value &&
        (!std::is_convertible<M,Linear>::value ||
        !std::is_convertible<N,Linear>::value)>::type>
Affinity operator*(const M &m, const N &n) {
#ifdef XFORM_DEBUG
        std::cerr << "operator*(static_cast<Affinity>, static_cast<Affinity>)\n";
#endif
    return static_cast<Affinity>(n).transformed(static_cast<Affinity>(m));
}

template <typename M, typename N,
    typename = typename std::enable_if<
        rvg::meta::is_an_ixform<M>::value &&
        rvg::meta::is_an_ixform<N>::value &&
        !std::is_same<M, N>::value &&
        std::is_convertible<M,Linear>::value &&
        std::is_convertible<N,Linear>::value>::type>
Linear operator*(const M &m, const N &n) {
#ifdef XFORM_DEBUG
    std::cerr << "Linear operator*(static_cast<Linear>, static_cast<Linear>)\n";
#endif
    return static_cast<Linear>(n).transformed(static_cast<Linear>(m));
}

template <typename M,
    typename = typename std::enable_if<
        rvg::meta::is_an_ixform<M>::value &&
        !std::is_same<M, Identity>::value>::type>
M operator*(const M &m, const Identity &) {
#ifdef XFORM_DEBUG
        std::cerr << "operator*(const IXform &, const Identity &)\n";
#endif
    return m;
}

template <typename M,
    typename = typename std::enable_if<
        rvg::meta::is_an_ixform<M>::value &&
        !std::is_same<M, Identity>::value>::type>
M operator*(const Identity &, const M &m) {
#ifdef XFORM_DEBUG
        std::cerr << "operator*(const IXform &, const Identity &)\n";
#endif
    return m;
}

template <typename M,
    typename = typename std::enable_if<
        rvg::meta::is_an_ixform<M>::value>::type>
decltype(std::declval<M&>().scaled(float()))
operator*(float s, const M &m)
{
#ifdef XFORM_DEBUG
        std::cerr << "operator*(float s, const IXform &)\n";
#endif
    return m.scaled(s);
}
