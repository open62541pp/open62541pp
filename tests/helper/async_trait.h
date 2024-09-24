#pragma once

#include <type_traits>

template <typename, typename = void>
struct IsTrait : std::false_type {};

template <typename T>
struct IsTrait<T, std::void_t<typename T::type>> : std::true_type {};

/**
 * Simple trait to mark template types for async execution.
 */
template <typename T>
struct Async {
    using type = T;
};

template <typename>
struct IsAsync : std::false_type {};

template <typename T>
struct IsAsync<Async<T>> : std::true_type {};

template <typename T>
constexpr bool isAsync = IsAsync<T>::value;
