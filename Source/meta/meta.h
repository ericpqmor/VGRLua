#ifndef RVG_META_H
#define RVG_META_H

#include <type_traits>

namespace rvg {
    namespace meta {

template <typename From, typename To>
struct is_same_or_convertible:
    std::integral_constant<bool,
        std::is_same<From,To>::value ||
        std::is_convertible<From, To>::value> {};

template <typename T>
struct remove_reference_cv {
    typedef typename
        std::remove_reference<typename
            std::remove_cv<T>::type>::type type;
};

template <typename From, typename To>
struct forward_same_or_convertible:
    std::integral_constant<
        bool,
        is_same_or_convertible<
            typename remove_reference_cv<From>::type, To>::value> {};

template <typename T, typename... REST>
struct is_all_same_or_convertible: std::true_type {};

template<typename T, typename FIRST, typename... REST>
struct is_all_same_or_convertible<T, FIRST, REST...>:
  std::integral_constant<bool,
    is_same_or_convertible<T, FIRST>::value &&
    is_all_same_or_convertible<T, REST...>::value> {};

template <template<typename> typename Base, typename Derived>
struct is_crtp_of {
    static constexpr bool value = std::is_base_of<Base<Derived>, Derived>::value;
};

// ??D This will not be needed once we have c++14.
// Should use std::integer_sequence instead.
template<size_t... Is> struct sequence{
    constexpr size_t size(void) const { return sizeof...(Is); }
    using type = sequence;
};

template<size_t N1, class S1, class S2> struct concat;

template<size_t N1, size_t... I1, size_t... I2>
struct concat<N1, sequence<I1...>, sequence<I2...>>
  : sequence<I1..., (N1+I2)...>{};

template<size_t N1, class S1, class S2>
using concat_t = typename concat<N1, S1, S2>::type;

template<size_t N> struct make_sequence;

template<size_t N> using make_sequence_t = typename make_sequence<N>::type;

template<size_t N>
struct make_sequence : concat_t<N/2, make_sequence_t<N/2>, make_sequence_t<N - N/2>>{};

template<> struct make_sequence<0>: sequence<>{};
template<> struct make_sequence<1>: sequence<0>{};

} } // namespace rvg::meta

#endif
