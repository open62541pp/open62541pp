#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>  // move

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Types.h"
#include "open62541pp/open62541.h"

namespace opcua {

/**
 * Template base class to wrap UA_* type objects.
 *
 * The derived classes should implement specific constructors to convert from other data types.
 */
template <typename T, Type type = detail::getType<T>()>
class TypeWrapper {
public:
    using BaseClass = TypeWrapper<T, type>;
    using UaType = T;

    TypeWrapper() {
        init();
    }

    /// Constructor with native UA_* type (deep copy)
    explicit TypeWrapper(const T& data) {
        set(data);
    }

    /// Constructor with native UA_* type (move rvalue)
    explicit TypeWrapper(T&& data) noexcept
        : data_(data) {}

    virtual ~TypeWrapper() {
        clear();
    };

    /// Copy constructor (deep copy)
    TypeWrapper(const TypeWrapper& other) {
        set(other.data_);
    }

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
    constexpr static Type getType() {
        return type;
    }

    /// Return UA_DataType object
    constexpr static const UA_DataType* getDataType() {
        return detail::getUaDataType(type);
    }

    /// Return pointer to wrapped UA data type
    T* handle() noexcept {
        return &data_;
    }

    /// Return const pointer to wrapped UA data type
    const T* handle() const noexcept {
        return &data_;
    };

protected:
    void init() noexcept {
        UA_init(&data_, getDataType());
    }

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

/* ---------------------------------------------------------------------------------------------- */

/**
 * UA_String wrapper class.
 */
class String : public TypeWrapper<UA_String> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    explicit String(std::string_view str);

    bool operator==(const String& other) const noexcept;
    bool operator!=(const String& other) const noexcept;

    std::string get() const;
    std::string_view getView() const;
};

/**
 * UA_Guid wrapper class.
 */
class Guid : public TypeWrapper<UA_Guid> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    Guid(UA_UInt32 data1, UA_UInt16 data2, UA_UInt16 data3, std::array<UA_Byte, 8> data4);

    bool operator==(const Guid& other) const noexcept;
    bool operator!=(const Guid& other) const noexcept;
};

/**
 * UA_ByteString wrapper class.
 *
 * UA_ByteString is a typedef of UA_String. Therefore the type can not be deduced from UA_ByteString
 * itself. Second template argument (Type::ByteString) necessary to map to the right type.
 */
class ByteString : public TypeWrapper<UA_ByteString, Type::ByteString> {
public:
    // NOLINTNEXTLINE, false positive?
    using BaseClass::BaseClass;  // inherit contructors

    explicit ByteString(std::string_view str);

    static ByteString fromBase64(std::string_view base64);

    bool operator==(const ByteString& other) const noexcept;
    bool operator!=(const ByteString& other) const noexcept;

    std::string get() const;
    std::string_view getView() const;
};

/**
 * UA_QualifiedName wrapper class.
 */
class QualifiedName : public TypeWrapper<UA_QualifiedName> {
public:
    // NOLINTNEXTLINE, false positive?
    using BaseClass::BaseClass;  // inherit contructors

    QualifiedName(uint16_t namespaceIndex, std::string_view name);

    bool operator==(const QualifiedName& other) const noexcept;
    bool operator!=(const QualifiedName& other) const noexcept;

    uint16_t getNamespaceIndex() const noexcept;
    std::string getName() const;
    std::string_view getNameView() const;
};

/**
 * UA_LocalizedText wrapper class.
 */
class LocalizedText : public TypeWrapper<UA_LocalizedText> {
public:
    // NOLINTNEXTLINE, false positive?
    using BaseClass::BaseClass;  // inherit contructors

    LocalizedText(std::string_view text, std::string_view locale);

    bool operator==(const LocalizedText& other) const noexcept;
    bool operator!=(const LocalizedText& other) const noexcept;

    std::string getText() const;
    std::string_view getTextView() const;
    std::string getLocale() const;
    std::string_view getLocaleView() const;
};

}  // namespace opcua
