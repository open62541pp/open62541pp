#pragma once

#include <iterator>
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

template <typename Iterator>
struct IsMutableIterator
    : std::negation<std::is_const<
          std::remove_reference_t<typename std::iterator_traits<Iterator>::reference>>> {};

template <typename T>
struct IsMutableContainer
    : std::conjunction<
          IsMutableIterator<decltype(std::declval<T>().begin())>,
          IsMutableIterator<decltype(std::declval<T>().end())>> {};

}  // namespace opcua::detail
