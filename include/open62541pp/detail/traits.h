#pragma once

#include <type_traits>

namespace opcua::detail {

constexpr bool isConstantEvaluated() noexcept {
#if __cpp_lib_is_constant_evaluated >= 201811L
    return std::is_constant_evaluated();
#else
    return false;
#endif
}

template <typename...>
struct AlwaysFalse : std::false_type {};

template <typename T, typename... Ts>
using IsOneOf = std::disjunction<std::is_same<T, Ts>...>;

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
