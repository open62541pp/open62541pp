#pragma once

#include <array>
#include <algorithm> // copy
#include <type_traits>

#include "open62541pp/Types.h"
#include "open62541pp/Helper.h"
#include "open62541pp/ErrorHandling.h"

namespace opcua {

class TypeWrapperBase {
public:
    virtual ~TypeWrapperBase(){};
};

/**
 * Template base class to wrap UA_* type objects.
 * 
 * The derived classes should implement specific constructors to convert from other data types.
 */
template <typename T, Type type = getType<T>()>
class TypeWrapper : public TypeWrapperBase {
public:
    using BaseClass = TypeWrapper<T, type>;
    using UaType = T;

    TypeWrapper() { init(); }

    /// Constructor with native UA_* type (deep copy)
    explicit TypeWrapper(const T& data) { init(); copy(data); }
    /// Constructor with native UA_* type (move rvalue)
    explicit TypeWrapper(T&& data) : data_(data) {}

    virtual ~TypeWrapper() { clear(); };

    /// Copy constructor (deep copy)
    TypeWrapper(const TypeWrapper& other)            { clear(); copy(other.data_); }
    /// Copy assignment (deep copy)
    TypeWrapper& operator=(const TypeWrapper& other) { clear(); copy(other.data_); return *this; }

    /// Return type enum
    constexpr inline static Type               getType()     { return type; }
    /// Return UA_DataType object
    constexpr inline static const UA_DataType* getDataType() { return getUaDataType(type); }

    /// Return pointer to wrapped UA data type
    inline T*       handle()       { return &data_; }
    /// Return const pointer to wrapped UA data type
    inline const T* handle() const { return &data_; };

protected:
    inline void init()  { UA_init(&data_, getDataType()); }
    inline void clear() { UA_clear(&data_, getDataType()); }

    /// Deep copy of data
    inline void copy(const T& data) {
        auto status = UA_copy(&data, &data_, getDataType());
        checkStatusCodeException(status);
    }

    T data_{};
};

// template <typename T>
// using isTypeWrapper = std::is_convertible<T*, TypeWrapper<T::UaType, T::getType()>*>;

/**
 * UA_String wrapper class
 */
class String : public TypeWrapper<UA_String> {
public:
    using BaseClass::BaseClass; // inherit contructors

    explicit String(std::string_view str) {
        data_ = allocUaString(str);
    }

    inline bool operator==(const String& other) const { return UA_String_equal(&data_, other.handle()); }
    inline bool operator!=(const String& other) const { return !operator==(other); }

    inline std::string      get() const { return uaStringToString(data_); }
    inline std::string_view getView() const { return uaStringToStringView(data_); }
};


/**
 * UA_Guid wrapper class
 */
class Guid : public TypeWrapper<UA_Guid> {
public:
    using BaseClass::BaseClass; // inherit contructors

    Guid(UA_UInt32 data1, UA_UInt16 data2, UA_UInt16 data3, std::array<UA_Byte, 8> data4) {
        data_.data1 = data1;
        data_.data2 = data2;
        data_.data3 = data3;
        std::copy(data4.begin(), data4.end(), data_.data4);
    }

    inline bool operator==(const Guid& other) const { return UA_Guid_equal(&data_, other.handle()); }
    inline bool operator!=(const Guid& other) const { return !operator==(other); }
};


/**
 * UA_ByteString wrapper class
 * 
 * UA_ByteString is a typedef of UA_String. Therefore the type can not be deduced from UA_ByteString itself.
 * Second template argument (Type::ByteString) necessary to map to the right type.
 */
class ByteString : public TypeWrapper<UA_ByteString, Type::ByteString> {
public:
    using BaseClass::BaseClass; // inherit contructors

    explicit ByteString(std::string_view str) {
        auto tmp = allocUaString(str);
        data_.data   = tmp.data;
        data_.length = tmp.length;
    }

    inline bool operator==(const ByteString& other) const { return UA_ByteString_equal(&data_, other.handle()); }
    inline bool operator!=(const ByteString& other) const { return !operator==(other); }

    inline std::string get() const { return uaStringToString(data_); }
};


/**
 * UA_QualifiedName wrapper class
 */
class QualifiedName : public TypeWrapper<UA_QualifiedName> {
public:
    using BaseClass::BaseClass; // inherit contructors

    QualifiedName(uint16_t namespaceIndex, std::string_view name) {
        data_.namespaceIndex = namespaceIndex;
        data_.name           = allocUaString(name);
    }

    inline bool operator==(const QualifiedName& other) const { return UA_QualifiedName_equal(&data_, other.handle()); }
    inline bool operator!=(const QualifiedName& other) const { return !operator==(other); }

    inline uint16_t    getNamespaceIndex() const { return data_.namespaceIndex; }
    inline std::string getName() const { return uaStringToString(data_.name); }
};


/**
 * UA_LocalizedText wrapper class
 */
class LocalizedText : public TypeWrapper<UA_LocalizedText> {
public:
    using BaseClass::BaseClass; // inherit contructors

    LocalizedText(std::string_view text, std::string_view locale = "") {
        data_.locale = allocUaString(locale);
        data_.text   = allocUaString(text);
    }

    inline std::string getText() const { return uaStringToString(data_.text); }
    inline std::string getLocale() const { return uaStringToString(data_.locale); }
};

} // namespace opcua
