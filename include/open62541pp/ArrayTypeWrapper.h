#pragma once

#include <vector>

#include "open62541pp/Helper.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"

namespace opcua {

/**
 * @defgroup ArrayTypeWrapper Wrapper classes of UA_* c-array types
 *
 * Safe wrapper classes for heap-allocated open62541 `UA_*` c-array types to prevent memory leaks.
 * @n
 * All array wrapper classes inherit from opcua::ArrayTypeWrapper.
 * Native open62541 objects can be accessed using the opcua::TypeWrapper::handle() method.
 * Elements can be accessed with the index operator [i], at(i), and for pointer elements
 * getPtrAt(i).
 */

/**
 * Template base class to wrap UA_* c-array type objects.
 *
 * The derived classes should implement specific constructors to convert from other data types.
 * @ingroup ArrayTypeWrapper
 */
template <typename T, uint16_t typeIndex>
class ArrayTypeWrapper {
public:
    static_assert(typeIndex < UA_TYPES_COUNT);

    using ArrayTypeWrapperBase = ArrayTypeWrapper<T, typeIndex>;
    using UaType = T;

    ArrayTypeWrapper()
        : size_(0),
          data_(nullptr) {}

    explicit ArrayTypeWrapper(std::size_t size)
        : size_(size) {
        init();
    }

    /// Constructor with native UA_* c-array type (deep copy).
    explicit ArrayTypeWrapper(const T* data, std::size_t size)
        : size_(size),
          data_(nullptr) {
        copy(data, size_);
    }

    /// Constructor with native UA_* c-array type (move rvalue).
    explicit ArrayTypeWrapper(T*&& data, std::size_t size) noexcept
        : size_(size),
          data_(data) {}

    virtual ~ArrayTypeWrapper() {
        clear();
    };

    /// Copy constructor (deep copy).
    ArrayTypeWrapper(const ArrayTypeWrapper& other) {
        copy(other.data_, other.size_);
    }

    /// Move constructor.
    ArrayTypeWrapper(ArrayTypeWrapper&& other) noexcept {
        swap(other);
    }

    /// Copy assignment (deep copy).
    ArrayTypeWrapper& operator=(const ArrayTypeWrapper& other) {  // NOLINT, false positive
        if (this == &other) {
            return *this;
        }
        copy(other.data_, other.size_);
        return *this;
    }

    /// Move assignment.
    ArrayTypeWrapper& operator=(ArrayTypeWrapper&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        swap(other);
        return *this;
    }

    /// Implicit conversion to wrapped object.
    operator T*() noexcept {  // NOLINT
        return data_;
    }

    /// Implicit conversion to wrapped object.
    operator const T*() const noexcept {  // NOLINT
        return data_;
    }

    /// Array access to element.
    TypeWrapper<T, typeIndex> getElement(int index) noexcept {
        return TypeWrapper<T, typeIndex>(data_[index]);
    }

    /// Array access to wrapped element.
    T operator[](int index) noexcept {
        return data_[index];
    }

    /// Array access to wrapped element.
    const T operator[](int index) const noexcept {
        return data_[index];
    }

    /// Array access to wrapped element with bounce check.
    T at(int index) {
        checkArrayIndex(index);
        return data_[index];
    }

    /// Array access to wrapped element with bounce check.
    const T at(int index) const {
        checkArrayIndex(index);
        return data_[index];
    }

    /// Array access to wrapped pointer element with bounce check.
    T* getPtrAt(int index) {
        checkArrayIndex(index);
        return &data_[index];
    }

    /// Array access to wrapped pointer element with bounce check.
    const T* getPtrAt(int index) const {
        checkArrayIndex(index);
        return &data_[index];
    }

    /// Swap wrapped objects.
    void swap(ArrayTypeWrapper& other) noexcept {
        static_assert(std::is_swappable_v<T>);
        std::swap(this->size_, other.size_);
        std::swap(this->data_, other.data_);
    }

    /// Get type as type index.
    static constexpr uint16_t getTypeIndex() {
        return typeIndex;
    }

    /// Get type as Type enum (only for builtin types).
    static constexpr Type getType() {
        static_assert(typeIndex < UA_TYPES_COUNT, "Only possible for builtin types");
        return static_cast<Type>(typeIndex);
    }

    /// Get type as UA_DataType object.
    static const UA_DataType* getDataType() {
        return detail::getUaDataType<typeIndex>();
    }

    /// Return pointer to wrapped object.
    T* handle() noexcept {
        return data_;
    }

    /// Return const pointer to wrapped object.
    const T* handle() const noexcept {
        return data_;
    };

    std::size_t size() const noexcept {
        return size_;
    };

protected:
    inline static void checkMemSize() {
        assert(sizeof(T) == getDataType()->memSize);  // NOLINT
    }

    inline void checkArrayIndex(int i) {
        assert(i + 1 <= size_);
    }

    void init() noexcept {
        checkMemSize();
        data_ = reinterpret_cast<T*>(UA_Array_new(size_, getDataType()));
    }

    void clear() noexcept {
        checkMemSize();
        if (data_ != nullptr) {
            UA_Array_delete(data_, size_, getDataType());
        }
    }

    void copy(const T* data, std::size_t size) {
        clear();
        checkMemSize();
        auto status = UA_Array_copy(
            data, size, (void**)&data_, getDataType()
        );  // deep copy of data
        size_ = size;
        detail::throwOnBadStatus(status);
    }

private:
    std::size_t size_;
    T* data_;
};

/**
 * UA_String* wrapper class.
 * @ingroup ArrayTypeWrapper
 */
class StringArray : public ArrayTypeWrapper<UA_String, UA_TYPES_STRING> {
public:
    using ArrayTypeWrapperBase::ArrayTypeWrapperBase;

    explicit StringArray(const std::vector<std::string>& vStr);

    std::vector<std::string> get() const;
};

}  // namespace opcua
