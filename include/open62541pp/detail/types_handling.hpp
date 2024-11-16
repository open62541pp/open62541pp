#pragma once

#include <algorithm>  // copy_n
#include <cassert>
#include <cstring>  // memcpy, memset
#include <memory>
#include <new>  // bad_alloc

#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/traits.hpp"  // IsOneOf
#include "open62541pp/exception.hpp"

namespace opcua::detail {

/* ------------------------------------ Generic type handling ----------------------------------- */

template <typename T>
constexpr bool isPointerFree = IsOneOf<  // NOLINT(modernize-type-traits)
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
constexpr bool isBorrowed(const T& /* unused */) noexcept {
    return false;
}

constexpr bool isBorrowed(const UA_Variant& native) noexcept {
    return native.storageType == UA_VARIANT_DATA_NODELETE;
}

constexpr bool isBorrowed(const UA_DataValue& native) noexcept {
    return native.value.storageType == UA_VARIANT_DATA_NODELETE;
}

constexpr bool isBorrowed(const UA_ExtensionObject& native) noexcept {
    return native.encoding == UA_EXTENSIONOBJECT_DECODED_NODELETE;
}

template <typename T>
constexpr bool isValidTypeCombination(const UA_DataType& type) {
    if constexpr (std::is_void_v<T>) {
        return true;  // allow type-erasure
    } else {
        return sizeof(T) == type.memSize;
    }
}

template <typename T>
constexpr void init(T& native) noexcept {
    // caution: braced initialization will only zero-initialize the first member of a union
    // use memset for unions or structs that may contain unions
    if constexpr (std::is_union_v<T> || std::is_class_v<T>) {
        std::memset(&native, 0, sizeof(T));
    } else {
        native = {};
    }
}

template <typename T>
constexpr void clear(T& native, const UA_DataType& type) noexcept {
    assert(isValidTypeCombination<T>(type));
    // NOLINTNEXTLINE(bugprone-branch-clone)
    if constexpr (isPointerFree<T>) {
        init(native);
    } else if (isBorrowed(native)) {
        init(native);
    } else {
        UA_clear(&native, &type);
    }
}

template <typename T>
void deallocate(T* native, const UA_DataType& type) noexcept {
    assert(isValidTypeCombination<T>(type));
    UA_delete(native, &type);
}

template <typename T>
[[nodiscard]] T* allocate(const UA_DataType& type) {
    assert(isValidTypeCombination<T>(type));
    auto* result = static_cast<T*>(UA_new(&type));
    if (result == nullptr) {
        throw std::bad_alloc();
    }
    return result;
}

template <typename T>
[[nodiscard]] auto allocateUniquePtr(const UA_DataType& type) {
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
void deallocateArray(T* array, size_t size, const UA_DataType& type) noexcept {
    assert(isValidTypeCombination<T>(type));
    UA_Array_delete(array, size, &type);
}

template <typename T>
[[nodiscard]] T* allocateArray(size_t size, const UA_DataType& type) {
    assert(isValidTypeCombination<T>(type));
    auto* result = static_cast<T*>(UA_Array_new(size, &type));
    if (result == nullptr) {
        throw std::bad_alloc();
    }
    return result;
}

template <typename T>
[[nodiscard]] auto allocateArrayUniquePtr(size_t size, const UA_DataType& type) {
    auto deleter = [&type, size](T* native) { deallocateArray(native, size, type); };
    return std::unique_ptr<T, decltype(deleter)>(allocateArray<T>(size, type), deleter);
}

template <typename T>
[[nodiscard]] T* copyArray(const T* src, size_t size, const UA_DataType& type) {
    T* dst = allocateArray<T>(size, type);
    if constexpr (!isPointerFree<T>) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        std::transform(src, src + size, dst, [&](const T& item) { return copy(item, type); });
    } else {
        std::copy_n(src, size, dst);
    }
    return dst;
}

template <typename T>
void resizeArray(T*& array, size_t& size, size_t newSize, const UA_DataType& type) {
    if (newSize == size) {
        return;
    }
    T* newArray = allocateArray<T>(newSize, type);
    std::memcpy(newArray, array, std::min(size, newSize) * sizeof(T));
    if (newSize > size) {
        std::memset(newArray + size, 0, newSize - size);  // NOLINT
    }
    deallocateArray<T>(array, size, type);
    array = newArray;
    size = newSize;
}

}  // namespace opcua::detail
