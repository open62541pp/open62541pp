#pragma once

#include <algorithm>  // copy_n
#include <cassert>
#include <memory>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/traits.h"  // IsOneOf

namespace opcua::detail {

/* ------------------------------------ Generic type handling ----------------------------------- */

template <typename T>
inline constexpr bool isPointerFree = IsOneOf<
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
    UA_StatusCode>::value;

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
    if constexpr (!isPointerFree<T>) {
        UA_clear(&native, &type);
    }
}

template <typename T>
inline void deallocate(T* native, const UA_DataType& type) noexcept {
    assert(isValidTypeCombination<T>(type));
    UA_delete(native, &type);
}

template <typename T>
[[nodiscard]] inline T* allocate(const UA_DataType& type) {
    assert(isValidTypeCombination<T>(type));
    auto* result = static_cast<T*>(UA_new(&type));
    if (result == nullptr) {
        throw std::bad_alloc();
    }
    return result;
}

template <typename T>
[[nodiscard]] inline auto allocateUniquePtr(const UA_DataType& type) {
    auto deleter = [&type](T* native) { deallocate(native, type); };
    return std::unique_ptr<T, decltype(deleter)>(allocate<T>(type), deleter);
}

template <typename T>
[[nodiscard]] constexpr T copy(const T& src, const UA_DataType& type) noexcept(isPointerFree<T>) {
    assert(isValidTypeCombination<T>(type));
    if constexpr (!isPointerFree<T>) {
        T dst;  // NOLINT, initialized in UA_copy function
        throwIfBad(UA_copy(&src, &dst, &type));
        return dst;
    } else {
        return src;
    }
}

/* ----------------------------------- Generic array handling ----------------------------------- */

template <typename T>
inline void deallocateArray(T* array, size_t size, const UA_DataType& type) noexcept {
    assert(isValidTypeCombination<T>(type));
    UA_Array_delete(array, size, &type);
}

template <typename T>
[[nodiscard]] inline T* allocateArray(size_t size, const UA_DataType& type) {
    assert(isValidTypeCombination<T>(type));
    auto* result = static_cast<T*>(UA_Array_new(size, &type));
    if (result == nullptr) {
        throw std::bad_alloc();
    }
    return result;
}

template <typename T>
[[nodiscard]] inline auto allocateArrayUniquePtr(size_t size, const UA_DataType& type) {
    auto deleter = [&type, size](T* native) { deallocateArray(native, size, type); };
    return std::unique_ptr<T, decltype(deleter)>(allocateArray<T>(size, type), deleter);
}

template <typename T>
[[nodiscard]] inline T* copyArray(const T* src, size_t size, const UA_DataType& type) {
    assert(isValidTypeCombination<T>(type));
    if constexpr (!isPointerFree<T>) {
        T* dst{};
        throwIfBad(UA_Array_copy(src, size, (void**)&dst, &type));  // NOLINT
        return dst;
    } else {
        T* dst = allocateArray<T>(size, type);
        std::copy_n(src, size, dst);
        return dst;
    }
}

}  // namespace opcua::detail
