#pragma once

#include <iterator>  // data, iterator_traits, size
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

template <typename T>
using IterCategoryT = typename std::iterator_traits<T>::iterator_category;

template <typename T>
using IterValueT = typename std::iterator_traits<T>::value_type;

template <typename T>
using RangeIteratorT = decltype(std::begin(std::declval<T&>()));

template <typename T>
using RangeValueT = typename std::iterator_traits<RangeIteratorT<T>>::value_type;

template <typename, typename = void>
struct HasBegin : std::false_type {};

template <typename T>
struct HasBegin<T, std::void_t<decltype(std::begin(std::declval<T&>()))>> : std::true_type {};

template <typename, typename = void>
struct HasEnd : std::false_type {};

template <typename T>
struct HasEnd<T, std::void_t<decltype(std::end(std::declval<T&>()))>> : std::true_type {};

template <typename, typename = void>
struct HasData : std::false_type {};

template <typename T>
struct HasData<T, std::void_t<decltype(std::data(std::declval<T&>()))>> : std::true_type {};

template <typename, typename = void>
struct HasDataPointer : std::false_type {};

template <typename T>
struct HasDataPointer<T, std::void_t<decltype(std::data(std::declval<T&>()))>>
    : std::is_pointer<decltype(std::data(std::declval<T&>()))> {};

template <typename, typename = void>
struct HasSize : std::false_type {};

template <typename T>
struct HasSize<T, std::void_t<decltype(std::size(std::declval<T&>()))>> : std::true_type {};

template <typename T>
struct IsRange : std::conjunction<HasBegin<T>, HasEnd<T>> {};

template <typename T>
struct IsContiguousRange : std::conjunction<IsRange<T>, HasDataPointer<T>, HasSize<T>> {};

template <typename T, typename = void>
struct IsStringLike : std::false_type {};

template <typename T>
struct IsStringLike<T, std::enable_if_t<IsRange<T>::value>>
    : std::conjunction<
          IsContiguousRange<T>,
          std::is_same<RangeValueT<T>, char>,
          std::is_constructible<std::remove_reference_t<T>, const char*>,
          std::is_constructible<std::remove_reference_t<T>, const char*, size_t>> {};

}  // namespace opcua::detail
