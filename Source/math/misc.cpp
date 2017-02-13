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

// A polynomial a_0 + a_1 x + a_2 x^2 + ... + a_N x^N
// is stored in the last N+1 elements of an array with S elements
// a_i is stored at position S-N-1+i
//
// Compute the derivative of polynomial, in place
template <size_t N, size_t S, size_t I, typename T>
void differentiate_helper(std::array<T, S> &coefs, rvg::meta::sequence<I>) {
    coefs[S-N+I+1] *= I+2;
}

template <size_t N, size_t S, size_t I, size_t... Is, typename T>
void differentiate_helper(std::array<T, S> &coefs, rvg::meta::sequence<I, Is...>) {
    coefs[S-N+I+1] *= I+2;
    differentiate_helper<N>(coefs, rvg::meta::sequence<Is...>());
}

template <size_t N, size_t S, typename T,
    typename = typename std::enable_if<(N > 1)>::type>
void differentiate(std::array<T, S> &coefs) {
    return differentiate_helper<N>(coefs, rvg::meta::make_sequence<N-1>{});
}

template <size_t N, size_t S, typename T,
    typename = typename std::enable_if<(N <= 1)>::type>
void differentiate(std::array<T, S> &, void * = nullptr) {
    ;
}

// Compute integral of polynomial, in place
//
template <size_t N, size_t S, size_t I, typename T>
void integrate_helper(std::array<T, S> &coefs, rvg::meta::sequence<I>) {
    coefs[S-N+I] /= I+2;
}

template <size_t N, size_t S, size_t I, size_t... Is, typename T>
void integrate_helper(std::array<T, S> &coefs, rvg::meta::sequence<I, Is...>) {
    coefs[S-N+I] /= I+2;
    integrate_helper<N>(coefs, rvg::meta::sequence<Is...>());
}

template <size_t N, size_t S, typename T,
    typename = typename std::enable_if<(N >= 1)>::type>
void integrate(std::array<T, S> &coefs) {
    return integrate_helper<N>(coefs, rvg::meta::make_sequence<N>{});
}

template <size_t N, size_t S, typename T,
    typename = typename std::enable_if<(N <= 0)>::type>
void integrate(std::array<T, S> &, void * = nullptr) {
    ;
}

template <size_t N, size_t I = 0, typename T,
    typename = typename std::enable_if<(I >= N-1)>::type>
T evaluate_fma(const std::array<T, N> &coefs, T, void * = nullptr) {
    return coefs[N-1];
}

template <size_t N, size_t I = 0, typename T,
    typename = typename std::enable_if<(I < N-1)>::type>
T evaluate_fma(const std::array<T, N> &coefs, T x) {
    return std::fma(x, evaluate<N,I+1>(coefs, x), coefs[I]);
}

template <typename T>
void two_sum(T a, T b, T &s, T &t) {
    s = a + b;
    T ap = s - b;
    T bp = s - a;
    T da = a - ap;
    T db = b - bp;
    t = da + db;
}

template <typename T>
void two_prod_fma(T a, T b, T &m, T &n) {
    m = a*b;
    n = std::fma(a, b, -m);
}

template <size_t N, typename T,
    typename = typename std::enable_if<(N == 1)>::type>
T evaluate_compensated(const std::array<T, N> &coefs, T, void * = nullptr) {
    return coefs[0];
}

template <size_t N, typename T,
    typename = typename std::enable_if<(N >= 2)>::type>
T evaluate_compensated(const std::array<T, N> &coefs, T x) {
    T ri = coefs[N-1];
    T r = T(0.);
    for (int i = N-2; i >= 0; i--) {
        T pi, ppi;
        two_prod_fma(ri, x, pi, ppi);
        T rri;
        two_sum(pi, coefs[i], ri, rri);
        T qi = ppi + rri;
        r = std::fma(r, x, qi);
    }
    return ri + r;
}

template <size_t N, size_t I = 0, typename T,
    typename = typename std::enable_if<(I > N-1)>::type>
T evaluate_pow(const std::array<T, N> &, T, void * = nullptr) {
    return T(0.);
}

template <size_t N, size_t I = 0, typename T,
    typename = typename std::enable_if<(I <= N-1)>::type>
T evaluate_pow(const std::array<T, N> &coefs, T x) {
    return std::pow(x, I)*coefs[I] + evaluate_pow<N,I+1>(coefs, x);
}

void test_differentiate_and_integrate(void) {
    std::cerr << "testing inplace polynomial differentiation and integration\n";
    std::array<float, 6> a = {1,1,1,1,1,1};
    differentiate<5>(a);
    unit_test((compare_polynomial<4>(a, std::array<float,5>{1,2,3,4,5})));
    differentiate<4>(a);
    unit_test((compare_polynomial<3>(a, std::array<float,4>{2,6,12,20})));
    differentiate<3>(a);
    unit_test((compare_polynomial<2>(a, std::array<float,3>{6,24,60})));
    differentiate<2>(a);
    unit_test((compare_polynomial<1>(a, std::array<float,2>{24,120})));
    differentiate<1>(a);
    unit_test((compare_polynomial<0>(a, std::array<float,1>{120})));
    integrate<0>(a);
    unit_test((compare_polynomial<1>(a, std::array<float,2>{24,120})));
    integrate<1>(a);
    unit_test((compare_polynomial<2>(a, std::array<float,3>{6,24,60})));
    integrate<2>(a);
    unit_test((compare_polynomial<3>(a, std::array<float,4>{2,6,12,20})));
    integrate<3>(a);
    unit_test((compare_polynomial<4>(a, std::array<float,5>{1,2,3,4,5})));
    integrate<4>(a);
    unit_test((compare_polynomial<5>(a, std::array<float,6>{1,1,1,1,1,1})));
}

