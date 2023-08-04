#pragma once

#include <tuple>
#include <type_traits>

namespace opcua::detail {

template <typename...>
struct AlwaysFalse : std::false_type {};

template <typename T, typename... Ts>
using IsOneOf = std::disjunction<std::is_same<T, Ts>...>;

template <typename, typename>
struct TupleHolds : std::false_type {};

template <typename... Ts, typename T>
struct TupleHolds<std::tuple<Ts...>, T> : std::bool_constant<(std::is_same_v<Ts, T> || ...)> {};

/// Remove pointer, references and all qualifiers.
template <typename T>
struct Unqualified {
    using type = std::remove_cv_t<std::remove_reference_t<std::remove_pointer_t<T>>>;
};

template <typename T>
using UnqualifiedT = typename Unqualified<T>::type;

/// Derive member type from member pointer
template <typename T>
struct MemberType;

template <typename C, typename T>
struct MemberType<T C::*> {
    using type = T;
};

template <typename T>
using MemberTypeT = typename MemberType<T>::type;

}  // namespace opcua::detail
