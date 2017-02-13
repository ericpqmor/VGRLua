#ifndef RVG_MATH_POLYNOMIAL_H
#define RVG_MATH_POLYNOMIAL_H

#include <array>

#include "meta/meta.h"

namespace rvg {
    namespace math {
        namespace polynomial {

// Return the derivative of the polynomial
//
// A polynomial a_0 + a_1 x + a_2 x^2 + ... + a_N x^N
// is stored in an array with N+1 elements with a_i stored at position i
//
template <size_t N, size_t... Is, typename T>
std::array<T, N-1> derivative_helper(const std::array<T, N> &coefs,
    rvg::meta::sequence<Is...>) {
    return std::array<T, N-1>{ coefs[Is+1]*(Is+1)... };
}

template <size_t N, typename T>
std::array<T, N-1> derivative(const std::array<T, N> &coefs) {
    return derivative_helper(coefs, rvg::meta::make_sequence<N-1>{});
}

// Evaluate polynomial in Horner form
//
// A polynomial a_0 + a_1 x + a_2 x^2 + ... + a_N x^N
// is stored in an array with N+1 elements with a_i stored at position i
//
template <size_t N, size_t I = 0, typename T,
    typename = typename std::enable_if<(I >= N-1)>::type>
T evaluate(const std::array<T, N> &coefs, T, void * = nullptr) {
    return coefs[N-1];
}

template <size_t N, size_t I = 0, typename T,
    typename = typename std::enable_if<(I < N-1)>::type>
T evaluate(const std::array<T, N> &coefs, T x) {
    return coefs[I]+x*evaluate<N,I+1>(coefs, x);
}

// Use bissection to narrow down on a root for f(x) = 0
// for x in the interval [a,b]. The values f(a) and f(b)
// are assumed *not* to be both positive or both negative,
// so there is at least one root in [a,b]. We assume a <= b.
//
template <typename F, typename T>
T bissect(F f, T a, T b) {
    T m = T(.5)*(a+b);
    if (m == a || m == b) return m;
    if (f(m) > T(0.)) {
        return bissect(f, a, m);
    } else {
        return bissect(f, m, b);
    }
}

template <size_t N, typename T>
bool refine(const std::array<T, N> &coefs, T a, T b, T &root) {
    auto f = [&coefs](T x) -> T { return evaluate(coefs, x); };
    float fa = f(a);
    if (fa == T(0)) {
        root = a;
        return true;
    }
    float fb = f(b);
    if (fb == T(0)) {
        root = b;
        return true;
    }
    if (fa < 0.f && fb > 0.f) {
        root = bissect(f, a, b);
        return true;
    }
    if (fa > 0.f && fb < 0.f) {
        root = bissect(f, b, a);
        return true;
    }
    return false;
}

template <size_t N, size_t M, typename T,
    typename = typename std::enable_if<(N == 2 && M > 2)>::type>
int find_roots(const std::array<T, N> &coefs, T a, T b,
    std::array<T, M> &roots, void * = nullptr) {
    roots[0] = a;
    T r = -coefs[0]/coefs[1];
    if (r >= a && r <= b) {
        roots[1] = r;
        roots[2] = b;
        return 1;
    } else {
        roots[1] = b;
        return 0;
    }
}

template <size_t N, size_t M, typename T,
    typename = typename std::enable_if<(N > 2 && M > N)>::type>
int find_roots(const std::array<T, N> &coefs, T a, T b,
    std::array<T, M> &roots) {
    std::array<T, N> critical;
    int ncritical = find_roots(derivative(coefs), a, b, critical);
    int nroots = 0;
    roots[nroots] = a;
    for (int i = 0; i <= ncritical; i++) {
        T root;
        if (refine(coefs, critical[i], critical[i+1], root)) {
            ++nroots;
            roots[nroots] = root;
        }
    }
    roots[nroots+1] = b;
    return nroots;
}

} } } // namespace rvg::math::polynomial

#endif
