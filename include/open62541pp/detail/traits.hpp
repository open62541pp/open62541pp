#pragma once

#include <iterator>  // data, size
#include <type_traits>

namespace opcua::detail {

template <typename... Ts>
struct Overload : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

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

template <typename, typename = void>
struct HasSize : std::false_type {};

template <typename T>
struct HasSize<T, std::void_t<decltype(std::size(std::declval<T&>()))>> : std::true_type {};

template <typename, typename = void>
struct HasData : std::false_type {};

template <typename T>
struct HasData<T, std::void_t<decltype(std::data(std::declval<T&>()))>> : std::true_type {};

template <typename, typename = void>
struct HasDataPointer : std::false_type {};

template <typename T>
struct HasDataPointer<T, std::void_t<decltype(std::data(std::declval<T&>()))>>
    : std::is_pointer<decltype(std::data(std::declval<T&>()))> {};

template <typename T, typename = void>
struct IsContainer : std::false_type {};

template <typename T>
struct IsContainer<
    T,
    std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>>
    : std::true_type {};

template <typename T>
inline constexpr bool isContainer = IsContainer<T>::value;

template <typename T>
struct IsContiguousContainer : std::conjunction<HasSize<T>, HasDataPointer<T>> {};

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
