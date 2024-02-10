#pragma once

#include <type_traits>

namespace opcua::detail {

template <typename...>
struct AlwaysFalse : std::false_type {};

template <typename T, typename... Ts>
using IsOneOf = std::disjunction<std::is_same<T, Ts>...>;

/// Derive member type from member pointer
template <typename T>
struct MemberType;

template <typename C, typename T>
struct MemberType<T C::*> {
    using type = T;
};

template <typename T>
using MemberTypeT = typename MemberType<T>::type;

template <typename T, typename = void>
struct IsContiguousContainer : std::false_type {};

template <typename T>
struct IsContiguousContainer<
    T,
    std::void_t<
        decltype(std::declval<T>().data()),
        decltype(std::declval<T>().size()),
        decltype(std::declval<T>().begin()),
        decltype(std::declval<T>().end())>>
    : std::is_pointer<decltype(std::declval<T>().data())>  // detect proxy iterator of vector<bool>
{};

template <typename T>
struct IsMutableContainer
    : std::conjunction<
          std::is_same<
              decltype(std::declval<T>().begin()),
              typename std::remove_reference_t<T>::iterator>,
          std::is_same<
              decltype(std::declval<T>().end()),
              typename std::remove_reference_t<T>::iterator>,
          std::negation<std::is_same<
              typename std::remove_reference_t<T>::iterator,
              typename std::remove_reference_t<T>::const_iterator>>> {};

}  // namespace opcua::detail
