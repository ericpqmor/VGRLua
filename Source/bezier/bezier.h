#ifndef RVG_BEZIER_H
#define RVG_BEZIER_H

#include <array>
#include <iostream>
#include "meta/meta.h"

namespace rvg {
    namespace bezier {

template <typename POINT, size_t DEGREE>
class Segment {

    std::array<POINT, DEGREE+1> m_p;

public:

    template <typename... ARGS,
        typename =  typename std::enable_if<
            rvg::meta::is_all_same_or_convertible<POINT, ARGS...>::value>::type>
    Segment(ARGS... args): m_p{{args...}} {
        static_assert(sizeof...(ARGS) == (DEGREE+1),
            "not enough control points");
    }

    Segment(void) = default;

    const POINT &operator[](int i) const { return m_p[i]; }
    POINT &operator[](int i) { return m_p[i]; }

    const POINT *begin(void) const { return &m_p[0]; }
    const POINT *end(void) const { return &m_p[DEGREE+1]; }

    constexpr size_t degree(void) const { return DEGREE; }

    std::ostream &print(std::ostream &out) const {
        out << "bezier" << DEGREE << '{';
        for (size_t i = 0; i < DEGREE; i++) {
            out << m_p[i] << ',';
        }
        out << m_p[DEGREE] << '}';
        return out;
    }
};

template <typename P1, typename... Ps>
Segment<P1, sizeof...(Ps)> make_segment(const P1 &p1, Ps...ps) {
    static_assert(rvg::meta::is_all_same_or_convertible<P1, Ps...>::value,
        "incompatible control point types");
    return Segment<P1, sizeof...(Ps)>{p1, ps...};
}

template <typename POINT, size_t DEGREE>
std::ostream &operator<<(std::ostream &out, const Segment<POINT, DEGREE> &S) {
    return S.print(out);
}

// Compute the derivative of a Bezier segment s(t) using N-1 muls and adds,
// where N is the degree of the curve segment
//
// Let S[i] be the ith control point, i = 0..N.  Then,
//
// S'[i] = N*(S[i+1]-S[i])
//
template <typename POINT, size_t DEGREE, size_t... Is>
Segment<POINT, DEGREE-1> derivative_helper(const Segment<POINT, DEGREE> &S,
    rvg::meta::sequence<Is...>) {
    static_assert(sizeof...(Is) == DEGREE,
        "auxiliary sequence is incompatible with arguments");
    return Segment<POINT,DEGREE-1>(DEGREE*(S[Is+1]-S[Is])...);
}

template <typename POINT, size_t DEGREE>
Segment<POINT, DEGREE-1> derivative(const Segment<POINT, DEGREE> &S) {
    return derivative_helper(S, rvg::meta::make_sequence<DEGREE>{});
}

// Evaluate Bezier segment s(t) by Horner's algorithm,
// using about 4N muls and N adds,
// where N is the degree of the curve segment
//
// Let S[i] be the ith control point, i = 0..N. Let u = 1-t.
// The idea is to compute
// s(t) = (...(S[0] choose(N,0) u + S[1] choose(N,1) t) u +
//   S[2] choose(N,2) t^2) u + S[3] choose(N, 3) t^3) u...) u
//      + S[N] choose(N,N) t^N
//
// We will incrementally compute values
//    p_k, for k = 0..N, so that in the end s(t) = p_N
//    c_k, for k = 0..N, so that c_k = choose(N, k)
//    t_k, for k = 0..N, so that t_k = t^k
//
// The basic case is
// p_0 = S[0];
// t_0 = 1.f;
// c_0 = 1;
//
// The iteartion is
// t_k = t_{k-1}*t,
// c_k = (c_{k-1}*(N-(k-1)))/k, and
// p_{k+1} = p_k*u + S[k]*c_k*t_k
//
// We want the compiler to expand all code in-line for
// us, without the need for any loop. So we first rewrite
// the iteration as a recursive function
//
// f(p_{k-1}, k, N, c_k, t_k, t, u)
//
// If k > N, we simply return p_{k-1}. Otherwise, we recursively
// invoke the function with p_k and other updated arguments
//
// f(p_{k-1}, k, N, c_k, t_k, t, u) :=
//   k > N ? p_{k-1}:
//     f(p_{k-1}*u + S[k]*c_k*t_k, k+1, N, (c_k*(N-k))/(k+1), t_k*t, t, u)
//
// To obtain s(t), we invoke
//
// f(p_0 = S[0], k = 1, N, t_1 = t, c_1 = N, t, u = 1-t)
//
// This helper template matches the end of the recursion, where we
// simply return p_k
//
template <size_t K, size_t CK, typename POINT, size_t DEGREE,
    typename = typename std::enable_if<(K > DEGREE)>::type>
POINT evaluate_helper(const Segment<POINT, DEGREE> &, const POINT &pk1,
    float, float, float, void * = nullptr) {
    return pk1;
}
//
// This helper template makes the instantiates the next
// step in the recursive computation
//
template <size_t K, size_t CK, typename POINT, size_t DEGREE,
    typename = typename std::enable_if<(K <= DEGREE)>::type>
POINT evaluate_helper(const Segment<POINT, DEGREE> &S, const POINT &pk1,
    float tk, float t, float u) {
    return evaluate_helper<K+1, (CK*(DEGREE-K))/(K+1)>(
        S, pk1*u + S[K]*static_cast<float>(CK)*tk, tk*t, t, u);
}
//
// This template simply calls the helper template with initial conditions
//
template <typename POINT, size_t DEGREE>
POINT evaluate(const Segment<POINT, DEGREE> &S, float t, float u) {
    return evaluate_helper<1, DEGREE>(S, S[0], t, t, u);
}

template <typename POINT, size_t DEGREE>
POINT evaluate(const Segment<POINT, DEGREE> &S, float t) {
    return evaluate_helper<1, DEGREE>(S, S[0], t, t, 1.f-t);
}

// Evaluate one step of the blossom for a Bezier curve at t.
// Let S[i] be the ith control point, i = 0..N. Let u = 1-t.
// We simply compute the control points R[j], j=0..N-1
// such that R[j] = S[j]*u + S[j+1]*t
//
template <typename POINT, size_t DEGREE, size_t... Is>
Segment<POINT, DEGREE-1> blossom_helper(const Segment<POINT, DEGREE> &S,
    float t, float u, rvg::meta::sequence<Is...>) {
    static_assert(sizeof...(Is) == DEGREE,
        "auxiliary sequence is incompatible with arguments");
    return Segment<POINT, DEGREE-1>{(u*S[Is]+t*S[Is+1])...};
}

template <typename POINT, size_t DEGREE>
Segment<POINT, DEGREE-1> blossom(const Segment<POINT, DEGREE> &S,
    float t, float u) {
    return blossom_helper(S, t, u, rvg::meta::make_sequence<DEGREE>{});
}

template <typename POINT, size_t DEGREE>
Segment<POINT, DEGREE-1> blossom(const Segment<POINT, DEGREE> &S, float t) {
    return blossom_helper(S, t, 1.f-t, rvg::meta::make_sequence<DEGREE>{});
}

// Subdivide a Bezier segment s(u) at u = t, using about
// (N-1)(N-2) muls and (N-1)(N-2)/2 adds
// where N is the degree of the curve segment
//
// Let S[i] be the ith control point, i = 0..N.
// Let S1 and S2 be the results of subdivision
//
// If S(u, u, u,..., u) = s(u) is the polar form of the segment,
// we know that
//
// S1[0] = S(0, 0, 0, ..., 0, 0, 0)
// S1[1] = S(t, 0, 0, ..., 0, 0, 0)
// S1[2] = S(t, t, 0, ..., 0, 0, 0)
// ...
// S1[N] = S(t, t, t, ..., t, t, t)
//
// S2[0] = S(t, t, t, ..., t, t, t)
// S2[1] = S(t, t, t, ..., t, t, 1)
// S2[2] = S(t, t, t, ..., t, 1, 1)
// ...
// S2[N] = S(1, 1, 1, ..., 1, 1, 1)
//
// S1[0] and S2[N] are the endpoints.
// Then, we apply one blossoming step at t.
// S1[1] and S2[N-1] are the resulting endpoints.
// Then, we apply another blossoming step at t.
// S1[2] and S2[N-2] are the resulting endpoints.
// And so on.
//
// This template helper matches the end of the recursion
//
template <size_t K, typename POINT, size_t DEGREE,
    typename = typename std::enable_if<(K >= DEGREE)>::type>
void split_helper(const Segment<POINT, 0> &S, float, float,
    Segment<POINT, DEGREE> &S1, Segment<POINT, DEGREE> &S2, void * = nullptr) {
    S1[DEGREE] = S2[0] = S[0];
}
//
// This template helper computes S1[K] and S2[N-k], then
// recursively invokes itself on the blossom.
//
template <size_t K, typename POINT, size_t DEGREE,
    typename = typename std::enable_if<(K < DEGREE)>::type>
void split_helper(const Segment<POINT, DEGREE-K> &S, float t, float u,
    Segment<POINT, DEGREE> &S1, Segment<POINT, DEGREE> &S2) {
    S1[K] = S[0];
    S2[DEGREE-K] = S[DEGREE-K];
    split_helper<K+1>(blossom(S, t, u), t, u, S1, S2);
}

template <typename POINT, size_t DEGREE>
void split(const Segment<POINT, DEGREE> &S, float t, float u,
    Segment<POINT, DEGREE> &S1, Segment<POINT, DEGREE> &S2) {
    return split_helper<0>(S, t, u, S1, S2);
}

template <typename POINT, size_t DEGREE>
void split(const Segment<POINT, DEGREE> &S, float t,
    Segment<POINT, DEGREE> &S1, Segment<POINT, DEGREE> &S2) {
    return split_helper<0>(S, t, 1.f-t, S1, S2);
}

// Cut a segment (or extend it) from [0,1] to [a,b]
// Works in a similar way to split
template <size_t K, typename POINT, size_t DEGREE,
    typename = typename std::enable_if<(K >= DEGREE)>::type>
void cut_helper(const Segment<POINT, 0> &S, float, float, float, float, Segment<POINT, DEGREE> &C, void * = nullptr) {
    C[DEGREE] = S[0];
}

template <size_t K, typename POINT, size_t DEGREE,
    typename = typename std::enable_if<(K < DEGREE)>::type>
void cut_helper(const Segment<POINT, DEGREE-K> &S, float a, float a1, float b, float b1, Segment<POINT, DEGREE> &C) {
    C[K] = evaluate(S, a, a1);
    cut_helper<K+1>(blossom(S, b, b1), a, a1, b, b1, C);
}

template <typename POINT, size_t DEGREE>
void cut(const Segment<POINT, DEGREE> &S, float a, float b, Segment<POINT, DEGREE> &C) {
    cut_helper<0>(S, a, 1.f-a, b, 1.f-b, C);
}

} } // namespace rvg::bezier

#endif
