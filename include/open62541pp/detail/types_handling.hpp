#pragma once

#include <algorithm>  // for_each_n, transform
#include <cassert>
#include <cstring>  // memcpy
#include <memory>
#include <new>  // bad_alloc
#include <type_traits>
#include <utility>  // forward

#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/traits.hpp"  // IsOneOf
#include "open62541pp/exception.hpp"

namespace opcua::detail {

/* ------------------------------------ Generic type handling ----------------------------------- */

template <typename T>
struct IsPointerFree
    : IsOneOf<
          T,
          UA_Boolean,
          UA_SByte,
          UA_Byte,
          UA_Int16,
          UA_UInt16,
          UA_Int32,
          UA_UInt32,
          UA_Int64,
          UA_UInt64,
          UA_Float,
          UA_Double,
          UA_DateTime,
          UA_Guid,
          UA_StatusCode> {};

template <typename T>
constexpr bool isValidTypeCombination(const UA_DataType& type) {
    if constexpr (std::is_void_v<T>) {
        return true;  // allow type-erasure
    } else {
        return sizeof(T) == type.memSize;
    }
}

template <typename T>
constexpr void clear(T& native, const UA_DataType& type) noexcept {
    assert(isValidTypeCombination<T>(type));
    if constexpr (IsPointerFree<T>::value) {
        native = {};
    } else {
        UA_clear(&native, &type);
    }
}

template <typename T>
void deallocate(T* native) noexcept {
    UA_free(native);  // NOLINT
}

template <typename T>
void deallocate(T* native, const UA_DataType& type) noexcept {
    assert(isValidTypeCombination<T>(type));
    if (native != nullptr) {
        clear(*native, type);
        deallocate(native);
    }
}

template <typename T>
[[nodiscard]] T* allocate() {
    auto* ptr = static_cast<T*>(UA_calloc(1, sizeof(T)));  // NOLINT
    if (ptr == nullptr) {
        throw std::bad_alloc{};
    }
    return ptr;
}

template <typename T>
[[nodiscard]] auto makeUnique(const UA_DataType& type) {
    auto deleter = [&type](T* ptr) { deallocate(ptr, type); };
    return std::unique_ptr<T, decltype(deleter)>(allocate<T>(), deleter);
}

template <typename T>
[[nodiscard]] constexpr T copy(const T& src, const UA_DataType& type) {
    assert(isValidTypeCombination<T>(type));
    if constexpr (IsPointerFree<T>::value) {
        return src;
    } else {
        T dst;  // NOLINT, initialized in UA_copy function
        throwIfBad(UA_copy(&src, &dst, &type));
        return dst;
    }
}

template <typename T>
[[nodiscard]] constexpr auto copy(T&& src, const UA_DataType& type) {
    if constexpr (std::is_rvalue_reference_v<decltype(src)>) {
        return std::forward<T>(src);
    } else {
        return copy(std::as_const(src), type);
    }
}

/* ----------------------------------- Generic array handling ----------------------------------- */

/**
 * Sentinel for empty but defined arrays.
 *
 * In OPC UA, arrays can have a length of zero or more with the usual meaning.
 * In addition, arrays can be undefined. Then, they don't even have a length.
 * In the binary encoding, this is indicated by an array of length -1.
 *
 * In open62541, size_t is used for array lengths.
 * An undefined array has length 0 and the data pointer is NULL.
 * An array of length 0 also has length 0 but a data pointer UA_EMPTY_ARRAY_SENTINEL (0x01).
 */
inline constexpr uintptr_t emptyArraySentinel = 0x01;

template <typename T>
[[nodiscard]] T* makeEmptyArraySentinel() noexcept {
    return reinterpret_cast<T*>(emptyArraySentinel);  // NOLINT
}

template <typename T>
[[nodiscard]] T* stripEmptyArraySentinel(T* array) noexcept {
    // NOLINTNEXTLINE
    return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(array) & ~emptyArraySentinel);
}

template <typename T>
void deallocateArray(T* array) noexcept {
    UA_free(stripEmptyArraySentinel(array));  // NOLINT
}

template <typename T>
void deallocateArray(T* array, size_t size, const UA_DataType& type) noexcept {
    assert(isValidTypeCombination<T>(type));
    std::for_each_n(array, size, [&](T& item) { clear(item, type); });
    deallocateArray(array);
}

template <typename T>
[[nodiscard]] T* allocateArray(size_t size) {
    if (size > UA_INT32_MAX) {
        throw std::bad_alloc{};
    }
    if (size == 0) {
        return makeEmptyArraySentinel<T>();
    }
    auto* ptr = static_cast<T*>(UA_calloc(size, sizeof(T)));  // NOLINT
    if (ptr == nullptr) {
        throw std::bad_alloc{};
    }
    return ptr;
}

template <typename InputIt>
[[nodiscard]] std::pair<IterValueT<InputIt>*, size_t> copyArray(
    InputIt first, InputIt last, const UA_DataType& type, std::forward_iterator_tag /* unused */
) {
    using ValueType = IterValueT<InputIt>;
    const size_t size = std::distance(first, last);
    auto* dst = allocateArray<ValueType>(size);
    try {
        std::transform(first, last, dst, [&](const ValueType& item) { return copy(item, type); });
    } catch (...) {
        deallocateArray(dst, size, type);
        throw;
    }
    return {dst, size};
}

template <typename InputIt>
[[nodiscard]] std::pair<IterValueT<InputIt>*, size_t> copyArray(
    InputIt first, InputIt last, const UA_DataType& type, std::input_iterator_tag /* unused */
) {
    using ValueType = IterValueT<InputIt>;
    if (first == last) {
        return {makeEmptyArraySentinel<ValueType>(), 0};
    }

    size_t index = 0;
    size_t capacity = 16;  // initial capacity to avoid frequency reallocations
    auto dst = allocateArray<ValueType>(capacity);

    const auto reallocate = [&](size_t newSize) {
        // resize without clearing members; newSize must be greater than number of members
        auto* ptr = static_cast<ValueType*>(UA_realloc(dst, newSize * sizeof(ValueType)));
        if (ptr == nullptr) {
            throw std::bad_alloc{};
        }
        dst = ptr;
    };

    try {
        for (auto it = first; it != last; ++it, ++index) {
            if (index >= capacity) {
                capacity *= 2;
                reallocate(capacity);
            }
            dst[index] = copy(*it, type);
        }
        reallocate(index);
    } catch (...) {
        deallocateArray(dst, index, type);
        throw;
    }
    return {dst, index};
}

template <typename InputIt>
[[nodiscard]] std::pair<IterValueT<InputIt>*, size_t> copyArray(
    InputIt first, InputIt last, const UA_DataType& type
) {
    return copyArray(first, last, type, IterCategoryT<InputIt>{});
}

template <typename T>
[[nodiscard]] T* copyArray(const T* src, size_t size) {
    if (stripEmptyArraySentinel(src) == nullptr || size == 0) {
        return makeEmptyArraySentinel<T>();
    }
    T* dst = allocateArray<T>(size);
    std::memcpy(dst, src, size * sizeof(T));
    return dst;
}

template <typename T>
[[nodiscard]] T* copyArray(const T* src, size_t size, const UA_DataType& type) {
    return IsPointerFree<T>::value
        ? copyArray(src, size)
        : copyArray(src, src + size, type).first;  // NOLINT
}

}  // namespace opcua::detail
