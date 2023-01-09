#pragma once

#include <algorithm>  // copy
#include <array>
#include <type_traits>
#include <utility>  // move

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/Types.h"

namespace opcua {

/**
 * Template base class to wrap UA_* type objects.
 *
 * The derived classes should implement specific constructors to convert from other data types.
 */
template <typename T, Type type = getType<T>()>
class TypeWrapper {
public:
    using BaseClass = TypeWrapper<T, type>;
    using UaType = T;

    TypeWrapper() { init(); }

    /// Constructor with native UA_* type (deep copy)
    explicit TypeWrapper(const T& data) { set(data); }

    /// Constructor with native UA_* type (move rvalue)
    explicit TypeWrapper(T&& data) noexcept : data_(data) {}

    virtual ~TypeWrapper() { clear(); };

    /// Copy constructor (deep copy)
    TypeWrapper(const TypeWrapper& other) { set(other.data_); }

    /// Move constructor
    TypeWrapper(TypeWrapper&& other) noexcept {
        set(std::move(other.data_));
        other.ownsData_ = false;
    }

    /// Copy assignment (deep copy)
    TypeWrapper& operator=(const TypeWrapper& other) {
        if (this == &other) {
            return *this;
        }
        set(other.data_);
        return *this;
    }

    /// Move assignment
    TypeWrapper& operator=(TypeWrapper&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        set(std::move(other.data_));
        other.ownsData_ = false;
        return *this;
    }

    /// Return type enum
    constexpr static Type getType() { return type; }

    /// Return UA_DataType object
    constexpr static const UA_DataType* getDataType() { return getUaDataType(type); }

    /// Return pointer to wrapped UA data type
    T* handle() { return &data_; }

    /// Return const pointer to wrapped UA data type
    const T* handle() const { return &data_; };

protected:
    void init() noexcept { UA_init(&data_, getDataType()); }

    void clear() noexcept {
        if (ownsData_) {
            UA_clear(&data_, getDataType());
        }
    }

    void set(const T& data) {
        clear();
        auto status = UA_copy(&data, &data_, getDataType());  // deep copy of data
        detail::checkStatusCodeException(status);
        ownsData_ = true;
    }

    void set(T&& data) {
        clear();
        data_ = data;
        ownsData_ = true;
    }

private:
    T data_{};
    bool ownsData_{true};
};

/* -------------------------------------------- Trait ------------------------------------------- */

namespace detail {

// https://stackoverflow.com/a/51910887
template <typename T, Type type>
std::true_type isTypeWrapperImpl(TypeWrapper<T, type>*);
std::false_type isTypeWrapperImpl(...);

template <typename T>
using IsTypeWrapper = decltype(isTypeWrapperImpl(std::declval<T*>()));

}  // namespace detail

/* --------------------------------------- Implementations -------------------------------------- */

/**
 * UA_String wrapper class.
 */
class String : public TypeWrapper<UA_String> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    explicit String(std::string_view str) : String(UA_String{detail::allocUaString(str)}) {}

    bool operator==(const String& other) const { return UA_String_equal(handle(), other.handle()); }

    bool operator!=(const String& other) const { return !operator==(other); }

    std::string get() const { return detail::toString(*handle()); }

    std::string_view getView() const { return detail::toStringView(*handle()); }
};

/**
 * UA_Guid wrapper class.
 */
class Guid : public TypeWrapper<UA_Guid> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    Guid(UA_UInt32 data1, UA_UInt16 data2, UA_UInt16 data3, std::array<UA_Byte, 8> data4)
        : Guid(UA_Guid{
              data1,
              data2,
              data3,
              {data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]},
          }) {}

    bool operator==(const Guid& other) const { return UA_Guid_equal(handle(), other.handle()); }

    bool operator!=(const Guid& other) const { return !operator==(other); }
};

/**
 * UA_ByteString wrapper class.
 *
 * UA_ByteString is a typedef of UA_String. Therefore the type can not be deduced from UA_ByteString
 * itself. Second template argument (Type::ByteString) necessary to map to the right type.
 */
class ByteString : public TypeWrapper<UA_ByteString, Type::ByteString> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    explicit ByteString(std::string_view str)
        : ByteString(UA_ByteString{detail::allocUaString(str)}) {}

    bool operator==(const ByteString& other) const {
        return UA_ByteString_equal(handle(), other.handle());
    }

    bool operator!=(const ByteString& other) const { return !operator==(other); }

    std::string get() const { return detail::toString(*handle()); }

    std::string_view getView() const { return detail::toStringView(*handle()); }
};

/**
 * UA_QualifiedName wrapper class.
 */
class QualifiedName : public TypeWrapper<UA_QualifiedName> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    QualifiedName(uint16_t namespaceIndex, std::string_view name)
        : QualifiedName(UA_QualifiedName{namespaceIndex, detail::allocUaString(name)}) {}

    bool operator==(const QualifiedName& other) const {
        return UA_QualifiedName_equal(handle(), other.handle());
    }

    bool operator!=(const QualifiedName& other) const { return !operator==(other); }

    uint16_t getNamespaceIndex() const { return handle()->namespaceIndex; }

    std::string getName() const { return detail::toString(handle()->name); }

    std::string_view getNameView() const { return detail::toStringView(handle()->name); }
};

/**
 * UA_LocalizedText wrapper class.
 */
class LocalizedText : public TypeWrapper<UA_LocalizedText> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    LocalizedText(std::string_view text, std::string_view locale = "")  // NOLINT
        : LocalizedText(UA_LocalizedText{detail::allocUaString(locale), detail::allocUaString(text)}
          ) {}

    std::string getText() const { return detail::toString(handle()->text); }

    std::string_view getTextView() const { return detail::toStringView(handle()->text); }

    std::string getLocale() const { return detail::toString(handle()->locale); }

    std::string_view getLocaleView() const { return detail::toStringView(handle()->locale); }
};

}  // namespace opcua
