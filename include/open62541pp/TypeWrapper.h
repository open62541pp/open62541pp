#pragma once

#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>  // move, swap

#include "open62541pp/Common.h"
#include "open62541pp/Comparison.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/open62541.h"

namespace opcua {

/**
 * @defgroup TypeWrapper Wrapper classes of UA_* types
 *
 * Safe wrapper classes for heap-allocated open62541 `UA_*` types to prevent memory leaks.
 * @n
 * All wrapper classes inherit from opcua::TypeWrapper.
 * Native open62541 objects can be accessed using the opcua::TypeWrapper::handle() method.
 */

/**
 * Template base class to wrap UA_* type objects.
 *
 * The derived classes should implement specific constructors to convert from other data types.
 * @ingroup TypeWrapper
 */
template <typename T, uint16_t typeIndex>
class TypeWrapper {
public:
    static_assert(typeIndex < UA_TYPES_COUNT);

    using TypeWrapperBase = TypeWrapper<T, typeIndex>;
    using UaType = T;

    TypeWrapper() {
        init();
    }

    /// Constructor with native UA_* type (deep copy).
    explicit TypeWrapper(const T& data) {
        copy(data);
    }

    /// Constructor with native UA_* type (move rvalue).
    explicit TypeWrapper(T&& data) noexcept
        : data_(data) {}

    virtual ~TypeWrapper() {
        clear();
    };

    /// Copy constructor (deep copy).
    TypeWrapper(const TypeWrapper& other) {
        copy(other.data_);
    }

    /// Move constructor.
    TypeWrapper(TypeWrapper&& other) noexcept {
        swap(other);
    }

    /// Copy assignment (deep copy).
    TypeWrapper& operator=(const TypeWrapper& other) {  // NOLINT, false positive
        if (this == &other) {
            return *this;
        }
        copy(other.data_);
        return *this;
    }

    /// Move assignment.
    TypeWrapper& operator=(TypeWrapper&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        swap(other);
        return *this;
    }

    /// Implicit conversion to wrapped object.
    operator T&() noexcept {  // NOLINT
        return data_;
    }

    /// Implicit conversion to wrapped object.
    operator const T&() const noexcept {  // NOLINT
        return data_;
    }

    /// Member access to wrapped object.
    T* operator->() noexcept {
        return &data_;
    }

    /// Member access to wrapped object.
    const T* operator->() const noexcept {
        return &data_;
    }

    /// Swap wrapped objects.
    void swap(TypeWrapper& other) noexcept {
        static_assert(std::is_swappable_v<T>);
        std::swap(this->data_, other.data_);
    }

    /// Get type as type index.
    static constexpr uint16_t getTypeIndex() {
        return typeIndex;
    }

    /// Get type as Type enum (only for builtin types).
    static constexpr Type getType() {
        static_assert(typeIndex < UA_BUILTIN_TYPES_COUNT, "Only possible for builtin types");
        return static_cast<Type>(typeIndex);
    }

    /// Get type as UA_DataType object.
    static const UA_DataType* getDataType() {
        return detail::getUaDataType<typeIndex>();
    }

    /// Return pointer to wrapped object.
    T* handle() noexcept {
        return &data_;
    }

    /// Return const pointer to wrapped object.
    const T* handle() const noexcept {
        return &data_;
    };

protected:
    inline static void checkMemSize() {
        assert(sizeof(T) == getDataType()->memSize);  // NOLINT
    }

    void init() noexcept {
        checkMemSize();
        UA_init(&data_, getDataType());
    }

    void clear() noexcept {
        checkMemSize();
        UA_clear(&data_, getDataType());
    }

    void copy(const T& data) {
        clear();
        checkMemSize();
        auto status = UA_copy(&data, &data_, getDataType());  // deep copy of data
        detail::throwOnBadStatus(status);
    }

private:
    T data_{};
};

/* -------------------------------------------- Trait ------------------------------------------- */

namespace detail {

// https://stackoverflow.com/a/51910887
template <typename T, uint16_t typeIndex>
std::true_type isTypeWrapperImpl(TypeWrapper<T, typeIndex>*);
std::false_type isTypeWrapperImpl(...);

template <typename T>
using IsTypeWrapper = decltype(isTypeWrapperImpl(std::declval<T*>()));

}  // namespace detail

/* ----------------------------------------- Comparison ----------------------------------------- */

// generate from UA_* type comparison

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator==(const T& left, const T& right) noexcept {
    return (*left.handle() == *right.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator!=(const T& left, const T& right) noexcept {
    return (*left.handle() != *right.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator<(const T& left, const T& right) noexcept {
    return (*left.handle() < *right.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator>(const T& left, const T& right) noexcept {
    return (*left.handle() > *right.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator<=(const T& left, const T& right) noexcept {
    return (*left.handle() <= *right.handle());
}

template <typename T, typename = std::enable_if_t<detail::IsTypeWrapper<T>::value>>
inline bool operator>=(const T& left, const T& right) noexcept {
    return (*left.handle() >= *right.handle());
}

/* ---------------------------------------------------------------------------------------------- */

/**
 * UA_String wrapper class.
 * @ingroup TypeWrapper
 */
class String : public TypeWrapper<UA_String, UA_TYPES_STRING> {
public:
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    explicit String(std::string_view str);

    std::string get() const;
    std::string_view getView() const;
};

/**
 * UA_Guid wrapper class.
 * @ingroup TypeWrapper
 */
class Guid : public TypeWrapper<UA_Guid, UA_TYPES_GUID> {
public:
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    Guid(UA_UInt32 data1, UA_UInt16 data2, UA_UInt16 data3, std::array<UA_Byte, 8> data4);
};

/**
 * UA_ByteString wrapper class.
 * @ingroup TypeWrapper
 */
class ByteString : public TypeWrapper<UA_ByteString, UA_TYPES_BYTESTRING> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    explicit ByteString(std::string_view str);

    std::string get() const;
    std::string_view getView() const;
};

/**
 * UA_XmlElement wrapper class.
 * @ingroup TypeWrapper
 */
class XmlElement : public TypeWrapper<UA_XmlElement, UA_TYPES_XMLELEMENT> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    explicit XmlElement(std::string_view str);

    std::string get() const;
    std::string_view getView() const;
};

/**
 * UA_QualifiedName wrapper class.
 * @ingroup TypeWrapper
 */
class QualifiedName : public TypeWrapper<UA_QualifiedName, UA_TYPES_QUALIFIEDNAME> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    QualifiedName(uint16_t namespaceIndex, std::string_view name);

    uint16_t getNamespaceIndex() const noexcept;
    std::string getName() const;
    std::string_view getNameView() const;
};

/**
 * UA_LocalizedText wrapper class.
 * @ingroup TypeWrapper
 */
class LocalizedText : public TypeWrapper<UA_LocalizedText, UA_TYPES_LOCALIZEDTEXT> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    LocalizedText(std::string_view text, std::string_view locale);

    std::string getText() const;
    std::string_view getTextView() const;
    std::string getLocale() const;
    std::string_view getLocaleView() const;
};

/**
 * UA_DateTime wrapper class.
 *
 * An instance in time. A DateTime value is encoded as a 64-bit signed integer which represents the
 * number of 100 nanosecond intervals since January 1, 1601 (UTC).
 *
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.2.2.5
 * @ingroup TypeWrapper
 */
class DateTime : public TypeWrapper<UA_DateTime, UA_TYPES_DATETIME> {
public:
    using DefaultClock = std::chrono::system_clock;
    using UaDuration = std::chrono::duration<uint64_t, std::ratio<1, 10'000'000>>;

    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    template <typename Clock, typename Duration>
    DateTime(std::chrono::time_point<Clock, Duration> timePoint)  // NOLINT, implicit wanted
        : DateTime(fromTimePoint(timePoint)) {}

    /// Get current DateTime.
    static DateTime now();

    /// Get DateTime from std::chrono::time_point.
    template <typename Clock, typename Duration>
    static DateTime fromTimePoint(std::chrono::time_point<Clock, Duration> timePoint);

    /// Get DateTime from Unix time.
    static DateTime fromUnixTime(uint64_t unixTime);

    /// Convert to std::chrono::time_point.
    template <typename Clock = DefaultClock, typename Duration = UaDuration>
    std::chrono::time_point<Clock, Duration> toTimePoint() const;

    /// Convert to Unix time.
    uint64_t toUnixTime() const noexcept;

    /// Convert to UA_DateTimeStruct.
    UA_DateTimeStruct toStruct() const;

    /// Get DateTime value as 100 nanosecond intervals since January 1, 1601 (UTC).
    uint64_t get() const noexcept;
};

template <typename Clock, typename Duration>
DateTime DateTime::fromTimePoint(std::chrono::time_point<Clock, Duration> timePoint) {
    static constexpr uint64_t dateTimeUnixEpoch = UA_DATETIME_UNIX_EPOCH;
    const uint64_t sinceUnixEpoch =
        std::chrono::duration_cast<UaDuration>(timePoint.time_since_epoch()).count();
    return DateTime(dateTimeUnixEpoch + sinceUnixEpoch);
}

template <typename Clock, typename Duration>
std::chrono::time_point<Clock, Duration> DateTime::toTimePoint() const {
    static constexpr std::chrono::time_point<Clock, Duration> unixEpoch{};
    const auto dateTime = get();
    if (dateTime < UA_DATETIME_UNIX_EPOCH) {
        return unixEpoch;
    }
    return unixEpoch + UaDuration(dateTime - UA_DATETIME_UNIX_EPOCH);
}

}  // namespace opcua
