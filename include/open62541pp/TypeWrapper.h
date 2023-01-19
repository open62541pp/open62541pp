#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>  // move, swap

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/Types.h"
#include "open62541pp/open62541.h"

namespace opcua {

/**
 * Template base class to wrap UA_* type objects.
 *
 * The derived classes should implement specific constructors to convert from other data types.
 */
template <typename T, Type type>
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
        swap(other);
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
        swap(other);
        return *this;
    }

    /// Swap wrapper objects
    void swap(TypeWrapper& other) noexcept {
        static_assert(std::is_swappable_v<UaType>);
        std::swap(this->data_, other.data_);
        std::swap(this->ownsData_, other.ownsData_);
    }

    /// Return type enum
    static constexpr Type getType() {
        return type;
    }

    /// Return UA_DataType object
    static const UA_DataType* getDataType() {
        return detail::getUaDataType<type>();
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
class String : public TypeWrapper<UA_String, Type::String> {
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
class Guid : public TypeWrapper<UA_Guid, Type::Guid> {
public:
    using BaseClass::BaseClass;  // inherit contructors

    Guid(UA_UInt32 data1, UA_UInt16 data2, UA_UInt16 data3, std::array<UA_Byte, 8> data4);

    bool operator==(const Guid& other) const noexcept;
    bool operator!=(const Guid& other) const noexcept;
};

/**
 * UA_ByteString wrapper class.
 */
class ByteString : public TypeWrapper<UA_ByteString, Type::ByteString> {
public:
    // NOLINTNEXTLINE, false positive?
    using BaseClass::BaseClass;  // inherit contructors

    explicit ByteString(std::string_view str);

    bool operator==(const ByteString& other) const noexcept;
    bool operator!=(const ByteString& other) const noexcept;

    std::string get() const;
    std::string_view getView() const;
};

/**
 * UA_XmlElement wrapper class.
 */
class XmlElement : public TypeWrapper<UA_XmlElement, Type::XmlElement> {
public:
    // NOLINTNEXTLINE, false positive?
    using BaseClass::BaseClass;  // inherit contructors

    explicit XmlElement(std::string_view str);

    bool operator==(const XmlElement& other) const noexcept;
    bool operator!=(const XmlElement& other) const noexcept;

    std::string get() const;
    std::string_view getView() const;
};

/**
 * UA_QualifiedName wrapper class.
 */
class QualifiedName : public TypeWrapper<UA_QualifiedName, Type::QualifiedName> {
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
class LocalizedText : public TypeWrapper<UA_LocalizedText, Type::LocalizedText> {
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

/**
 * UA_DateTime wrapper class.
 *
 * An instance in time. A DateTime value is encoded as a 64-bit signed integer which represents the
 * number of 100 nanosecond intervals since January 1, 1601 (UTC).
 *
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.2.2.5
 */
class DateTime : public TypeWrapper<UA_DateTime, Type::DateTime> {
public:
    using DefaultClock = std::chrono::system_clock;
    using UaDuration = std::chrono::duration<uint64_t, std::ratio<1, 10'000'000>>;

    // NOLINTNEXTLINE, false positive?
    using BaseClass::BaseClass;  // inherit contructors

    template <typename Clock, typename Duration>
    DateTime(std::chrono::time_point<Clock, Duration> timePoint)  // NOLINT, implicit wanted
        : DateTime(fromTimePoint(timePoint)) {}

    bool operator==(const DateTime& other) const noexcept;
    bool operator!=(const DateTime& other) const noexcept;
    bool operator<(const DateTime& other) const noexcept;
    bool operator>(const DateTime& other) const noexcept;

    /// Get current DateTime.
    static DateTime now();

    /// Get DateTime from std::chrono::time_point.
    template <typename Clock, typename Duration>
    static DateTime fromTimePoint(std::chrono::time_point<Clock, Duration> timePoint) {
        static constexpr uint64_t dateTimeUnixEpoch = UA_DATETIME_UNIX_EPOCH;
        const uint64_t sinceUnixEpoch =
            std::chrono::duration_cast<UaDuration>(timePoint.time_since_epoch()).count();
        return DateTime(dateTimeUnixEpoch + sinceUnixEpoch);
    }

    /// Get DateTime from Unix time.
    static DateTime fromUnixTime(uint64_t unixTime);

    /// Convert to std::chrono::time_point.
    template <typename Clock = DefaultClock, typename Duration = UaDuration>
    std::chrono::time_point<Clock, Duration> toTimePoint() const {
        static constexpr std::chrono::time_point<Clock, Duration> unixEpoch{};
        const auto dateTime = get();
        if (dateTime < UA_DATETIME_UNIX_EPOCH) {
            return unixEpoch;
        }
        return unixEpoch + UaDuration(dateTime - UA_DATETIME_UNIX_EPOCH);
    }

    /// Convert to Unix time.
    uint64_t toUnixTime() const noexcept;

    /// Convert to UA_DateTimeStruct.
    UA_DateTimeStruct toStruct() const;

    /// Get DateTime value as 100 nanosecond intervals since January 1, 1601 (UTC).
    uint64_t get() const noexcept;
};

}  // namespace opcua
