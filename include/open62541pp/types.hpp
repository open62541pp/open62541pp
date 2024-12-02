#pragma once

#include <algorithm>  // copy
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>  // hash
#include <initializer_list>
#include <iosfwd>  // forward declare ostream
#include <iterator>  // reverse_iterator
#include <optional>
#include <ratio>
#include <string>
#include <string_view>
#include <type_traits>  // is_same_v
#include <utility>  // move
#include <variant>
#include <vector>

#include "open62541pp/common.hpp"  // NamespaceIndex
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/string_utils.hpp"  // allocNativeString
#include "open62541pp/detail/traits.hpp"
#include "open62541pp/exception.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/typeconverter.hpp"
#include "open62541pp/typeregistry.hpp"
#include "open62541pp/typewrapper.hpp"
#include "open62541pp/wrapper.hpp"

namespace opcua {

/* ----------------------------------------- StatusCode ----------------------------------------- */

/**
 * UA_StatusCode wrapper class.
 * StatusCode can be used interchangeably with @ref UA_StatusCode due to implicit conversions
 * (without any overhead) but provides some methods to simplify the handling with status codes.
 * @see statuscodes.h
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.39
 * @ingroup Wrapper
 */
class StatusCode : public Wrapper<UA_StatusCode> {
public:
    /// Create a StatusCode with the default status code `UA_STATUSCODE_GOOD`.
    constexpr StatusCode() noexcept = default;

    constexpr StatusCode(UA_StatusCode code) noexcept  // NOLINT(hicpp-explicit-conversions)
        : Wrapper(code) {}

    /// Explicitly get underlying UA_StatusCode.
    constexpr UA_StatusCode get() const noexcept {
        return native();
    }

    /// Get human-readable name of the StatusCode.
    /// This feature might be disabled to create a smaller binary with the
    /// `UA_ENABLE_STATUSCODE_DESCRIPTIONS` build-flag. Then the function returns an empty string
    /// for every StatusCode.
    std::string_view name() const noexcept {
        return {UA_StatusCode_name(native())};
    }

    /// Check if the status code is good.
    constexpr bool isGood() const noexcept {
        return detail::isGood(native());
    }

    /// Check if the status code is uncertain.
    constexpr bool isUncertain() const noexcept {
        return detail::isUncertain(native());
    }

    /// Check if the status code is bad.
    constexpr bool isBad() const noexcept {
        return detail::isBad(native());
    }

    /// Throw a BadStatus exception if the status code is bad.
    /// @exception BadStatus
    constexpr void throwIfBad() const {
        opcua::throwIfBad(native());
    }
};

/* --------------------------------------- StringLikeMixin -------------------------------------- */

namespace detail {

/**
 * CRTP mixin class to provide a STL-compatible string interface.
 */
template <typename WrapperType, typename CharT>
class StringLikeMixin {
public:
    // clang-format off
    using value_type             = CharT;
    using size_type              = size_t;
    using difference_type        = std::ptrdiff_t;
    using pointer                = value_type*;
    using const_pointer          = const value_type*;
    using reference              = value_type&;
    using const_reference        = const value_type&;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    // clang-format on

    size_t size() const noexcept {
        const auto& native = asNative(static_cast<const WrapperType&>(*this));
        return native.length;
    }

    size_t length() const noexcept {
        return size();
    }

    bool empty() const noexcept {
        return size() == 0;
    }

    pointer data() noexcept {
        auto& native = asNative(static_cast<WrapperType&>(*this));
        return reinterpret_cast<pointer>(native.data);  // NOLINT
    }

    const_pointer data() const noexcept {
        const auto& native = asNative(static_cast<const WrapperType&>(*this));
        return reinterpret_cast<const_pointer>(native.data);  // NOLINT
    }

    reference operator[](size_t index) noexcept {
        assert(index < size());
        return data()[index];
    }

    const_reference operator[](size_t index) const noexcept {
        assert(index < size());
        return data()[index];
    }

    reference front() noexcept {
        assert(!empty());
        return *data();
    }

    const_reference front() const noexcept {
        assert(!empty());
        return *data();
    }

    reference back() noexcept {
        assert(!empty());
        return *(data() + size() - 1);
    }

    const_reference back() const noexcept {
        assert(!empty());
        return *(data() + size() - 1);
    }

    iterator begin() noexcept {
        return {data()};
    }

    const_iterator begin() const noexcept {
        return {data()};
    }

    const_iterator cbegin() const noexcept {
        return {data()};
    }

    iterator end() noexcept {
        return {data() + size()};
    }

    const_iterator end() const noexcept {
        return {data() + size()};
    }

    const_iterator cend() const noexcept {
        return {data() + size()};
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

protected:
    void init(size_t length) {
        auto& native = asNative(static_cast<WrapperType&>(*this));
        native.length = length;
        if (length > 0) {
            native.data = static_cast<uint8_t*>(UA_malloc(length));  // NOLINT
            if (data() == nullptr) {
                throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
            }
        }
    }

    template <typename InputIt>
    void init(InputIt first, InputIt last) {
        init(first, last, typename std::iterator_traits<InputIt>::iterator_category{});
    }

    template <typename InputIt, typename Tag>
    void init(InputIt first, InputIt last, Tag /* unused */) {
        init(std::distance(first, last));
        std::copy(first, last, data());
    }

    template <typename InputIt>
    void init(InputIt first, InputIt last, std::input_iterator_tag /* unused */) {
        // input iterator can only be read once -> buffer data in vector
        std::vector<uint8_t> buffer(first, last);
        init(buffer.size());
        std::copy(buffer.begin(), buffer.end(), data());
    }
};

}  // namespace detail

/* ------------------------------------------- String ------------------------------------------- */

/**
 * UA_String wrapper class.
 * @ingroup Wrapper
 */
class String
    : public TypeWrapper<UA_String, UA_TYPES_STRING>,
      public detail::StringLikeMixin<String, char> {
public:
    using TypeWrapper::TypeWrapper;

    explicit String(std::string_view str)
        : TypeWrapper(detail::allocNativeString(str)) {}

    template <typename InputIt>
    String(InputIt first, InputIt last) {
        init(first, last);
    }

    String(std::initializer_list<char> values) {
        init(values.begin(), values.end());
    }

    /// Implicit conversion to std::string_view.
    operator std::string_view() const noexcept {  // NOLINT(hicpp-explicit-conversions)
        return {data(), size()};
    }

    /// @deprecated Use conversion with `static_cast` instead
    [[deprecated("use conversion with static_cast instead")]]
    std::string_view get() const noexcept {
        return {data(), size()};
    }
};

inline bool operator==(const UA_String& lhs, const UA_String& rhs) noexcept {
    return UA_String_equal(&lhs, &rhs);
}

inline bool operator!=(const UA_String& lhs, const UA_String& rhs) noexcept {
    return !(lhs == rhs);
}

inline bool operator==(const String& lhs, std::string_view rhs) noexcept {
    return (static_cast<std::string_view>(lhs) == rhs);
}

inline bool operator!=(const String& lhs, std::string_view rhs) noexcept {
    return (static_cast<std::string_view>(lhs) != rhs);
}

inline bool operator==(std::string_view lhs, const String& rhs) noexcept {
    return (lhs == static_cast<std::string_view>(rhs));
}

inline bool operator!=(std::string_view lhs, const String& rhs) noexcept {
    return (lhs != static_cast<std::string_view>(rhs));
}

std::ostream& operator<<(std::ostream& os, const String& str);

template <>
struct TypeConverter<std::string_view> {
    using ValueType = std::string_view;
    using NativeType = String;

    static void fromNative(const NativeType& src, std::string_view& dst) {
        dst = static_cast<std::string_view>(src);
    }

    static void toNative(std::string_view src, NativeType& dst) {
        dst = String(src);
    }
};

template <>
struct TypeConverter<std::string> {
    using ValueType = std::string;
    using NativeType = String;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = std::string(src);
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        dst = String(src);
    }
};

template <>
struct TypeConverter<const char*> {
    using ValueType = const char*;
    using NativeType = String;

    static void toNative(const char* src, NativeType& dst) {
        dst = String(src);
    }
};

template <size_t N>
struct TypeConverter<char[N]> {  // NOLINT
    using ValueType = char[N];  // NOLINT
    using NativeType = String;

    static void toNative(const ValueType& src, NativeType& dst) {
        dst = String({static_cast<const char*>(src), N});
    }
};

/* ------------------------------------------ DateTime ------------------------------------------ */

/**
 * UA_DateTime wrapper class.
 *
 * An instance in time. A DateTime value is encoded as a 64-bit signed integer which represents the
 * number of 100 nanosecond intervals since January 1, 1601 (UTC).
 *
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.2.2.5
 * @ingroup Wrapper
 */
class DateTime : public TypeWrapper<UA_DateTime, UA_TYPES_DATETIME> {
public:
    using DefaultClock = std::chrono::system_clock;
    using UaDuration = std::chrono::duration<int64_t, std::ratio<1, 10'000'000>>;

    using TypeWrapper::TypeWrapper;  // inherit constructors

    template <typename Clock, typename Duration>
    DateTime(std::chrono::time_point<Clock, Duration> timePoint)  // NOLINT(*-explicit-conversions)
        : DateTime(fromTimePoint(timePoint)) {}

    /// Get current DateTime.
    static DateTime now() noexcept {
        return DateTime(UA_DateTime_now());  // NOLINT
    }

    /// Get DateTime from std::chrono::time_point.
    template <typename Clock, typename Duration>
    static DateTime fromTimePoint(std::chrono::time_point<Clock, Duration> timePoint) {
        return DateTime(
            int64_t{UA_DATETIME_UNIX_EPOCH} +
            std::chrono::duration_cast<UaDuration>(timePoint.time_since_epoch()).count()
        );
    }

    /// Get DateTime from Unix time.
    static DateTime fromUnixTime(int64_t unixTime) noexcept {
        return DateTime(UA_DateTime_fromUnixTime(unixTime));  // NOLINT
    }

    /// Offset of local time to UTC.
    static int64_t localTimeUtcOffset() noexcept {
        return UA_DateTime_localTimeUtcOffset();
    }

    /// Convert to std::chrono::time_point.
    template <typename Clock = DefaultClock, typename Duration = UaDuration>
    std::chrono::time_point<Clock, Duration> toTimePoint() const {
        const std::chrono::time_point<Clock, Duration> unixEpoch{};
        if (get() < UA_DATETIME_UNIX_EPOCH) {
            return unixEpoch;
        }
        const auto sinceEpoch = UaDuration(get() - UA_DATETIME_UNIX_EPOCH);
        return unixEpoch + std::chrono::duration_cast<Duration>(sinceEpoch);
    }

    /// Convert to Unix time (number of seconds since January 1, 1970 UTC).
    int64_t toUnixTime() const noexcept {
        if (get() < UA_DATETIME_UNIX_EPOCH) {
            return 0;
        }
        return UA_DateTime_toUnixTime(get());
    }

    /// Convert to UA_DateTimeStruct.
    UA_DateTimeStruct toStruct() const noexcept {
        return UA_DateTime_toStruct(get());
    }

    /// Get DateTime value as 100 nanosecond intervals since January 1, 1601 (UTC).
    int64_t get() const noexcept {
        return *handle();
    }

    /// Convert to string with given format (same format codes as strftime).
    /// @see https://en.cppreference.com/w/cpp/chrono/c/strftime
    std::string format(std::string_view format, bool localtime = false) const;
};

template <typename Clock, typename Duration>
struct TypeConverter<std::chrono::time_point<Clock, Duration>> {
    using ValueType = std::chrono::time_point<Clock, Duration>;
    using NativeType = DateTime;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = src.toTimePoint<Clock, Duration>();
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        dst = DateTime::fromTimePoint(src);
    }
};

/* -------------------------------------------- Guid -------------------------------------------- */

/**
 * UA_Guid wrapper class.
 * @ingroup Wrapper
 */
class Guid : public TypeWrapper<UA_Guid, UA_TYPES_GUID> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    explicit Guid(std::array<uint8_t, 16> data) noexcept
        : Guid(UA_Guid{
              // NOLINTBEGIN(hicpp-signed-bitwise)
              static_cast<uint32_t>(
                  (data[0] << 24U) | (data[1] << 16U) | (data[2] << 8U) | data[3]
              ),
              static_cast<uint16_t>((data[4] << 8U) | data[5]),
              static_cast<uint16_t>((data[6] << 8U) | data[7]),
              // NOLINTEND(hicpp-signed-bitwise)
              {data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]},
          }) {}

    Guid(uint32_t data1, uint16_t data2, uint16_t data3, std::array<uint8_t, 8> data4) noexcept
        : Guid(UA_Guid{
              data1,
              data2,
              data3,
              {data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]},
          }) {}

    static Guid random() noexcept {
        return Guid(UA_Guid_random());  // NOLINT
    }

    std::string toString() const;
};

inline bool operator==(const UA_Guid& lhs, const UA_Guid& rhs) noexcept {
    return UA_Guid_equal(&lhs, &rhs);
}

inline bool operator!=(const UA_Guid& lhs, const UA_Guid& rhs) noexcept {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& os, const Guid& guid);

/* ----------------------------------------- ByteString ----------------------------------------- */

/**
 * UA_ByteString wrapper class.
 * @ingroup Wrapper
 */
class ByteString
    : public TypeWrapper<UA_String, UA_TYPES_STRING>,
      public detail::StringLikeMixin<ByteString, uint8_t> {
public:
    using TypeWrapper::TypeWrapper;

    explicit ByteString(std::string_view str)
        : TypeWrapper(detail::allocNativeString(str)) {}

    explicit ByteString(const char* str)  // required to avoid ambiguity
        : ByteString(std::string_view(str)) {}

    explicit ByteString(Span<const uint8_t> bytes) {
        init(bytes.begin(), bytes.end());
    }

    template <typename InputIt>
    ByteString(InputIt first, InputIt last) {
        init(first, last);
    }

    /// Parse ByteString from Base64 encoded string.
    /// @note Supported since open62541 v1.1
    static ByteString fromBase64(std::string_view encoded);

    /// Explicit conversion to std::string_view.
    explicit operator std::string_view() const noexcept {
        return {reinterpret_cast<const char*>(data()), size()};  // NOLINT
    }

    /// Convert to Base64 encoded string.
    /// @note Supported since open62541 v1.1
    std::string toBase64() const;

    /// @deprecated Use conversion with `static_cast` instead
    [[deprecated("use conversion with static_cast instead")]]
    std::string_view get() const noexcept {
        return {reinterpret_cast<const char*>(data()), size()};  // NOLINT
    }
};

inline bool operator==(const ByteString& lhs, std::string_view rhs) noexcept {
    return (static_cast<std::string_view>(lhs) == rhs);
}

inline bool operator!=(const ByteString& lhs, std::string_view rhs) noexcept {
    return (static_cast<std::string_view>(lhs) != rhs);
}

inline bool operator==(std::string_view lhs, const ByteString& rhs) noexcept {
    return (lhs == static_cast<std::string_view>(rhs));
}

inline bool operator!=(std::string_view lhs, const ByteString& rhs) noexcept {
    return (lhs != static_cast<std::string_view>(rhs));
}

/* ----------------------------------------- XmlElement ----------------------------------------- */

/**
 * UA_XmlElement wrapper class.
 * @ingroup Wrapper
 */
class XmlElement
    : public TypeWrapper<UA_String, UA_TYPES_STRING>,
      public detail::StringLikeMixin<XmlElement, char> {
public:
    using TypeWrapper::TypeWrapper;

    explicit XmlElement(std::string_view str)
        : TypeWrapper(detail::allocNativeString(str)) {}

    template <typename InputIt>
    XmlElement(InputIt first, InputIt last) {
        init(first, last);
    }

    XmlElement(std::initializer_list<char> values) {
        init(values.begin(), values.end());
    }

    /// Implicit conversion to std::string_view.
    operator std::string_view() const noexcept {  // NOLINT(hicpp-explicit-conversions)
        return {data(), size()};
    }

    /// @deprecated Use conversion with `static_cast` instead
    [[deprecated("use conversion with static_cast instead")]]
    std::string_view get() const noexcept {
        return {data(), size()};
    }
};

std::ostream& operator<<(std::ostream& os, const XmlElement& xmlElement);

/* ------------------------------------------- NodeId ------------------------------------------- */

namespace detail {
template <typename T, typename = void>
struct IsNodeIdEnum : std::false_type {};

template <typename T>
struct IsNodeIdEnum<T, std::void_t<decltype(getNamespace(std::declval<T>()))>> : std::true_type {};
}  // namespace detail

/**
 * NodeId types.
 * @see UA_NodeIdType
 */
enum class NodeIdType : uint8_t {
    Numeric = UA_NODEIDTYPE_NUMERIC,
    String = UA_NODEIDTYPE_STRING,
    Guid = UA_NODEIDTYPE_GUID,
    ByteString = UA_NODEIDTYPE_BYTESTRING,
};

/**
 * UA_NodeId wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.2
 * @ingroup Wrapper
 */
class NodeId : public TypeWrapper<UA_NodeId, UA_TYPES_NODEID> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    /// Create NodeId with numeric identifier.
    NodeId(NamespaceIndex namespaceIndex, uint32_t identifier) noexcept {
        handle()->namespaceIndex = namespaceIndex;
        handle()->identifierType = UA_NODEIDTYPE_NUMERIC;
        handle()->identifier.numeric = identifier;  // NOLINT
    }

    /// Create NodeId with String identifier.
    NodeId(NamespaceIndex namespaceIndex, std::string_view identifier) {
        handle()->namespaceIndex = namespaceIndex;
        handle()->identifierType = UA_NODEIDTYPE_STRING;
        handle()->identifier.string = detail::allocNativeString(identifier);  // NOLINT
    }

    /// Create NodeId with Guid identifier.
    NodeId(NamespaceIndex namespaceIndex, Guid identifier) noexcept {
        handle()->namespaceIndex = namespaceIndex;
        handle()->identifierType = UA_NODEIDTYPE_GUID;
        handle()->identifier.guid = identifier;  // NOLINT
    }

    /// Create NodeId with ByteString identifier.
    NodeId(NamespaceIndex namespaceIndex, ByteString identifier) noexcept {
        handle()->namespaceIndex = namespaceIndex;
        handle()->identifierType = UA_NODEIDTYPE_BYTESTRING;
        // force zero-initialization, only first member of identifier union is zero-initialized
        handle()->identifier.byteString = {};  // NOLINT
        asWrapper<ByteString>(handle()->identifier.byteString) = std::move(identifier);  // NOLINT
    }

    /// Create NodeId from enum class with numeric identifiers like `opcua::ObjectId`.
    /// The namespace is retrieved by calling e.g. `getNamespace(opcua::ObjectId)`.
    /// Make sure to provide an overload for custom enum types.
    template <typename T, typename = std::enable_if_t<detail::IsNodeIdEnum<T>::value>>
    NodeId(T identifier) noexcept  // NOLINT(hicpp-explicit-conversions)
        : NodeId(getNamespace(identifier).index, static_cast<uint32_t>(identifier)) {}

    bool isNull() const noexcept {
        return UA_NodeId_isNull(handle());
    }

    uint32_t hash() const noexcept {
        return UA_NodeId_hash(handle());
    }

    NamespaceIndex namespaceIndex() const noexcept {
        return handle()->namespaceIndex;
    }

    /// @deprecated Use namespaceIndex() instead
    [[deprecated("use namespaceIndex() instead")]]
    NamespaceIndex getNamespaceIndex() const noexcept {
        return namespaceIndex();
    }

    NodeIdType identifierType() const noexcept {
        return static_cast<NodeIdType>(handle()->identifierType);
    }

    /// @deprecated Use identifierType() instead
    [[deprecated("use identifierType() instead")]]
    NodeIdType getIdentifierType() const noexcept {
        return identifierType();
    }

    /**
     * Get identifier pointer or `nullptr` on error.
     * @tparam T Requested identifier type, should be either:
     *         - `uint32_t` for NodeIdType::Numeric
     *         - `String` for NodeIdType::String
     *         - `Guid` for NodeIdType::Guid
     *         - `ByteString` for NodeIdType::ByteString
     * @return Pointer to the stored identifier
     *         or a `nullptr` if `T` doesn't match the stored identifier type
     */
    template <typename T>
    T* identifierIf() noexcept {
        return const_cast<T*>(std::as_const(*this).identifierIf<T>());  // NOLINT
    }

    /// @copydoc identifierIf
    template <typename T>
    const T* identifierIf() const noexcept {
        static_assert(
            detail::IsOneOf<T, uint32_t, String, Guid, ByteString>::value,
            "Invalid type for NodeId identifier"
        );
        // NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
        if constexpr (std::is_same_v<T, uint32_t>) {
            return identifierType() == NodeIdType::Numeric
                ? &handle()->identifier.numeric
                : nullptr;
        }
        if constexpr (std::is_same_v<T, String>) {
            return identifierType() == NodeIdType::String
                ? asWrapper<String>(&handle()->identifier.string)
                : nullptr;
        }
        if constexpr (std::is_same_v<T, Guid>) {
            return identifierType() == NodeIdType::Guid
                ? asWrapper<Guid>(&handle()->identifier.guid)
                : nullptr;
        }
        if constexpr (std::is_same_v<T, ByteString>) {
            return identifierType() == NodeIdType::ByteString
                ? asWrapper<ByteString>(&handle()->identifier.byteString)
                : nullptr;
        }
        // NOLINTEND(cppcoreguidelines-pro-type-union-access)
        return nullptr;
    }

    /**
     * Get identifier reference.
     * @tparam T Requested identifier type, should be either:
     *         - `uint32_t` for NodeIdType::Numeric
     *         - `String` for NodeIdType::String
     *         - `Guid` for NodeIdType::Guid
     *         - `ByteString` for NodeIdType::ByteString
     * @return Reference to the stored identifier
     * @throws TypeError If the requested type `T` doesn't match the stored identifier type
     */
    template <typename T>
    T& identifier() {
        return const_cast<T&>(std::as_const(*this).identifier<T>());  // NOLINT
    }

    /// @copydoc identifier
    template <typename T>
    const T& identifier() const {
        if (auto* ptr = identifierIf<T>()) {
            return *ptr;
        }
        throw TypeError("NodeId identifier type doesn't match the requested type");
    }

    /// Get identifier variant.
    /// @deprecated Use identifier<T>() or identifierIf<T>() instead
    [[deprecated("use identifier<T>() or identifierIf<T>() instead")]]
    std::variant<uint32_t, String, Guid, ByteString> getIdentifier() const {
        return getIdentifierImpl();
    }

    /// Get identifier by template type.
    /// @deprecated Use identifier<T>() or identifierIf<T>() instead
    template <typename T>
    [[deprecated("use identifier<T>() or identifierIf<T>() instead")]]
    auto getIdentifierAs() const {
        return getIdentifierAsImpl<T>();
    }

    /// Get identifier by NodeIdType enum.
    /// @deprecated Use identifier<T>() or identifierIf<T>() instead
    template <NodeIdType E>
    [[deprecated("use identifier<T>() or identifierIf<T>() instead")]]
    auto getIdentifierAs() const {
        return getIdentifierAsImpl<E>();
    }

    /// Encode NodeId as a string like `ns=1;s=SomeNode`.
    /// @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.3.1.10
    std::string toString() const;

private:
    std::variant<uint32_t, String, Guid, ByteString> getIdentifierImpl() const {
        switch (handle()->identifierType) {
        case UA_NODEIDTYPE_NUMERIC:
            return handle()->identifier.numeric;  // NOLINT
        case UA_NODEIDTYPE_STRING:
            return String(handle()->identifier.string);  // NOLINT
        case UA_NODEIDTYPE_GUID:
            return Guid(handle()->identifier.guid);  // NOLINT
        case UA_NODEIDTYPE_BYTESTRING:
            return ByteString(handle()->identifier.byteString);  // NOLINT
        default:
            return {};
        }
    }

    template <typename T>
    auto getIdentifierAsImpl() const {
        return std::get<T>(getIdentifierImpl());
    }

    template <NodeIdType E>
    auto getIdentifierAsImpl() const {
        if constexpr (E == NodeIdType::Numeric) {
            return getIdentifierAsImpl<uint32_t>();
        }
        if constexpr (E == NodeIdType::String) {
            return getIdentifierAsImpl<String>();
        }
        if constexpr (E == NodeIdType::Guid) {
            return getIdentifierAsImpl<Guid>();
        }
        if constexpr (E == NodeIdType::ByteString) {
            return getIdentifierAsImpl<ByteString>();
        }
    }
};

inline bool operator==(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return UA_NodeId_equal(&lhs, &rhs);
}

inline bool operator!=(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return !(lhs == rhs);
}

inline bool operator<(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return UA_NodeId_order(&lhs, &rhs) == UA_ORDER_LESS;
}

inline bool operator>(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return UA_NodeId_order(&lhs, &rhs) == UA_ORDER_MORE;
}

inline bool operator<=(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return (lhs < rhs) || (lhs == rhs);
}

inline bool operator>=(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return (lhs > rhs) || (lhs == rhs);
}

/* --------------------------------------- ExpandedNodeId --------------------------------------- */

/**
 * UA_ExpandedNodeId wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.16
 * @ingroup Wrapper
 */
class ExpandedNodeId : public TypeWrapper<UA_ExpandedNodeId, UA_TYPES_EXPANDEDNODEID> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    explicit ExpandedNodeId(NodeId id) noexcept {
        asWrapper<NodeId>(handle()->nodeId) = std::move(id);
    }

    ExpandedNodeId(NodeId id, std::string_view namespaceUri, uint32_t serverIndex) {
        asWrapper<NodeId>(handle()->nodeId) = std::move(id);
        handle()->namespaceUri = detail::allocNativeString(namespaceUri);
        handle()->serverIndex = serverIndex;
    }

    bool isLocal() const noexcept {
        return handle()->serverIndex == 0;
    }

    uint32_t hash() const noexcept {
        return UA_ExpandedNodeId_hash(handle());
    }

    NodeId& nodeId() noexcept {
        return asWrapper<NodeId>(handle()->nodeId);
    }

    const NodeId& nodeId() const noexcept {
        return asWrapper<NodeId>(handle()->nodeId);
    }

    /// @deprecated Use nodeId() instead
    [[deprecated("use nodeId() instead")]]
    NodeId& getNodeId() noexcept {
        return nodeId();
    }

    /// @deprecated Use nodeId() instead
    [[deprecated("use nodeId() instead")]]
    const NodeId& getNodeId() const noexcept {
        return nodeId();
    }

    std::string_view namespaceUri() const {
        return detail::toStringView(handle()->namespaceUri);
    }

    /// @deprecated Use namespaceUri() instead
    [[deprecated("use namespaceUri() instead")]]
    std::string_view getNamespaceUri() const {
        return namespaceUri();
    }

    uint32_t serverIndex() const noexcept {
        return handle()->serverIndex;
    }

    /// @deprecated Use serverIndex() instead
    [[deprecated("use serverIndex() instead")]]
    uint32_t getServerIndex() const noexcept {
        return serverIndex();
    }

    /// Encode ExpandedNodeId as a string like `svr=1;nsu=http://test.org/UA/Data/;ns=2;i=10157`.
    /// @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.3.1.11
    std::string toString() const;
};

inline bool operator==(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return UA_ExpandedNodeId_equal(&lhs, &rhs);
}

inline bool operator!=(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return !(lhs == rhs);
}

inline bool operator<(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return UA_ExpandedNodeId_order(&lhs, &rhs) == UA_ORDER_LESS;
}

inline bool operator>(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return UA_ExpandedNodeId_order(&lhs, &rhs) == UA_ORDER_MORE;
}

inline bool operator<=(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return (lhs < rhs) || (lhs == rhs);
}

inline bool operator>=(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return (lhs > rhs) || (lhs == rhs);
}

/* ---------------------------------------- QualifiedName --------------------------------------- */

/**
 * UA_QualifiedName wrapper class.
 * @ingroup Wrapper
 */
class QualifiedName : public TypeWrapper<UA_QualifiedName, UA_TYPES_QUALIFIEDNAME> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    QualifiedName(NamespaceIndex namespaceIndex, std::string_view name) {
        handle()->namespaceIndex = namespaceIndex;
        handle()->name = detail::allocNativeString(name);
    }

    NamespaceIndex namespaceIndex() const noexcept {
        return handle()->namespaceIndex;
    }

    /// @deprecated Use namespaceIndex() instead
    [[deprecated("use namespaceIndex() instead")]]
    NamespaceIndex getNamespaceIndex() const noexcept {
        return namespaceIndex();
    }

    std::string_view name() const noexcept {
        return detail::toStringView(handle()->name);
    }

    /// @deprecated Use name() instead
    [[deprecated("use name() instead")]]
    std::string_view getName() const noexcept {
        return name();
    }
};

inline bool operator==(const UA_QualifiedName& lhs, const UA_QualifiedName& rhs) noexcept {
    return (lhs.namespaceIndex == rhs.namespaceIndex) && (lhs.name == rhs.name);
}

inline bool operator!=(const UA_QualifiedName& lhs, const UA_QualifiedName& rhs) noexcept {
    return !(lhs == rhs);
}

/* ---------------------------------------- LocalizedText --------------------------------------- */

/**
 * UA_LocalizedText wrapper class.
 * The format of locale is `<language>[-<country/region>]`:
 * - `<language>` is the two letter ISO 639 code for a language
 * - `<country/region>` is the two letter ISO 3166 code for the country/region
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.5/
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.4/
 * @ingroup Wrapper
 */
class LocalizedText : public TypeWrapper<UA_LocalizedText, UA_TYPES_LOCALIZEDTEXT> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    LocalizedText(std::string_view locale, std::string_view text) {
        handle()->locale = detail::allocNativeString(locale);
        handle()->text = detail::allocNativeString(text);
    }

    std::string_view locale() const noexcept {
        return detail::toStringView(handle()->locale);
    }

    /// @deprecated Use locale() instead
    [[deprecated("use locale() instead")]]
    std::string_view getLocale() const noexcept {
        return locale();
    }

    std::string_view text() const noexcept {
        return detail::toStringView(handle()->text);
    }

    /// @deprecated Use text() instead
    [[deprecated("use text() instead")]]
    std::string_view getText() const noexcept {
        return text();
    }
};

inline bool operator==(const UA_LocalizedText& lhs, const UA_LocalizedText& rhs) noexcept {
    return (lhs.locale == rhs.locale) && (lhs.text == rhs.text);
}

inline bool operator!=(const UA_LocalizedText& lhs, const UA_LocalizedText& rhs) noexcept {
    return !(lhs == rhs);
}

/* ------------------------------------------- Variant ------------------------------------------ */

/**
 * Policies for variant factory methods Variant::fromScalar, Variant::fromArray.
 */
enum class VariantPolicy {
    // clang-format off
    Copy,                 ///< Store copy of scalar/array inside the variant.
    Reference,            ///< Store reference to scalar/array inside the variant.
                          ///< Both scalars and arrays must be mutable native/wrapper types.
                          ///< Arrays must store the elements contiguously in memory.
    ReferenceIfPossible,  ///< Favor referencing but fall back to copying if necessary.
    // clang-format on
};

namespace detail {
template <VariantPolicy>
struct VariantHandler;
}  // namespace detail

/**
 * UA_Variant wrapper class.
 * @ingroup Wrapper
 */
class Variant : public TypeWrapper<UA_Variant, UA_TYPES_VARIANT> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    /// Create Variant from scalar value.
    /// @tparam Policy Policy (@ref VariantPolicy) how to store the scalar inside the variant
    template <VariantPolicy Policy = VariantPolicy::Copy, typename T>
    [[nodiscard]] static Variant fromScalar(T&& value) {
        Variant var;
        detail::VariantHandler<Policy>::setScalar(var, std::forward<T>(value));
        return var;
    }

    /// Create Variant from scalar value with custom data type.
    /// @tparam Policy Policy (@ref VariantPolicy) how to store the scalar inside the variant
    template <VariantPolicy Policy = VariantPolicy::Copy, typename T>
    [[nodiscard]] static Variant fromScalar(T&& value, const UA_DataType& dataType) {
        Variant var;
        detail::VariantHandler<Policy>::setScalar(var, std::forward<T>(value), dataType);
        return var;
    }

    /// Create Variant from array.
    /// @tparam Policy Policy (@ref VariantPolicy) how to store the array inside the variant
    template <VariantPolicy Policy = VariantPolicy::Copy, typename ArrayLike>
    [[nodiscard]] static Variant fromArray(ArrayLike&& array) {
        using Handler = detail::VariantHandler<Policy>;
        Variant var;
        if constexpr (detail::IsContiguousContainer<ArrayLike>::value) {
            Handler::setArray(var, Span{std::forward<ArrayLike>(array)});
        } else {
            Handler::setArray(var, array.begin(), array.end());
        }
        return var;
    }

    /// Create Variant from array with custom data type.
    /// @tparam Policy Policy (@ref VariantPolicy) how to store the array inside the variant
    template <VariantPolicy Policy = VariantPolicy::Copy, typename ArrayLike>
    [[nodiscard]] static Variant fromArray(ArrayLike&& array, const UA_DataType& dataType) {
        using Handler = detail::VariantHandler<Policy>;
        Variant var;
        if constexpr (detail::IsContiguousContainer<ArrayLike>::value) {
            Handler::setArray(var, Span{std::forward<ArrayLike>(array)}, dataType);
        } else {
            Handler::setArray(var, array.begin(), array.end(), dataType);
        }
        return var;
    }

    /// Create Variant from range of elements (copy required).
    /// @tparam Policy Policy (@ref VariantPolicy) how to store the array inside the variant
    template <VariantPolicy Policy = VariantPolicy::Copy, typename InputIt>
    [[nodiscard]] static Variant fromArray(InputIt first, InputIt last) {
        Variant var;
        detail::VariantHandler<Policy>::setArray(var, first, last);
        return var;
    }

    /// Create Variant from range of elements with custom data type (copy required).
    /// @tparam Policy Policy (@ref VariantPolicy) how to store the array inside the variant
    template <VariantPolicy Policy = VariantPolicy::Copy, typename InputIt>
    [[nodiscard]] static Variant fromArray(
        InputIt first, InputIt last, const UA_DataType& dataType
    ) {
        Variant var;
        detail::VariantHandler<Policy>::setArray(var, first, last, dataType);
        return var;
    }

    /// Check if the variant is empty.
    bool isEmpty() const noexcept {
        return handle()->type == nullptr;
    }

    /// Check if the variant is a scalar.
    bool isScalar() const noexcept {
        return (
            !isEmpty() && handle()->arrayLength == 0 &&
            handle()->data > UA_EMPTY_ARRAY_SENTINEL  // NOLINT
        );
    }

    /// Check if the variant is an array.
    bool isArray() const noexcept {
        return !isEmpty() && !isScalar();
    }

    /// Check if the variant type is equal to the provided data type.
    bool isType(const UA_DataType* dataType) const noexcept {
        return (
            handle()->type != nullptr && dataType != nullptr &&
            handle()->type->typeId == dataType->typeId
        );
    }

    /// Check if the variant type is equal to the provided data type.
    bool isType(const UA_DataType& dataType) const noexcept {
        return isType(&dataType);
    }

    /// Check if the variant type is equal to the provided data type node id.
    bool isType(const NodeId& id) const noexcept {
        return (handle()->type != nullptr) && (handle()->type->typeId == id);
    }

    /// Check if the variant type is equal to the provided template type.
    template <typename T>
    bool isType() const noexcept {
        return isType(opcua::getDataType<T>());
    }

    /// Get data type.
    const UA_DataType* getDataType() const noexcept {
        return handle()->type;
    }

    /// Get pointer to the underlying data.
    /// Check the properties and data type before casting it to the actual type.
    /// Use the methods @ref isScalar, @ref isArray, @ref isType / @ref getDataType.
    void* data() noexcept {
        return handle()->data;
    }

    /// @copydoc data
    const void* data() const noexcept {
        return handle()->data;
    }

    /// Get reference to scalar value with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not a scalar or not of type `T`.
    template <typename T>
    T& getScalar() & {
        assertIsNative<T>();
        checkIsScalar();
        checkIsDataType<T>();
        return *static_cast<T*>(handle()->data);
    }

    /// @copydoc getScalar()&
    template <typename T>
    const T& getScalar() const& {
        assertIsNative<T>();
        checkIsScalar();
        checkIsDataType<T>();
        return *static_cast<const T*>(handle()->data);
    }

    /// @copydoc getScalar()&
    template <typename T>
    T&& getScalar() && {
        return std::move(getScalar<T>());
    }

    /// @copydoc getScalar()&
    template <typename T>
    const T&& getScalar() const&& {
        return std::move(getScalar<T>());
    }

    /// Get copy of scalar value with given template type.
    /// @exception BadVariantAccess If the variant is not a scalar or not convertible to `T`.
    template <typename T>
    T getScalarCopy() const {
        assertIsCopyableOrConvertible<T>();
        return getScalarCopyImpl<T>();
    }

    /// Get array length or 0 if variant is not an array.
    size_t getArrayLength() const noexcept {
        return handle()->arrayLength;
    }

    /// Get array dimensions.
    Span<const uint32_t> getArrayDimensions() const noexcept {
        return {handle()->arrayDimensions, handle()->arrayDimensionsSize};
    }

    /// Get array with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not an array or not of type `T`.
    template <typename T>
    Span<T> getArray() {
        assertIsNative<T>();
        checkIsArray();
        checkIsDataType<T>();
        return Span<T>(static_cast<T*>(handle()->data), handle()->arrayLength);
    }

    /// Get array with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not an array or not of type `T`.
    template <typename T>
    Span<const T> getArray() const {
        assertIsNative<T>();
        checkIsArray();
        checkIsDataType<T>();
        return Span<const T>(static_cast<const T*>(handle()->data), handle()->arrayLength);
    }

    /// Get copy of array with given template type and return it as a std::vector.
    /// @exception BadVariantAccess If the variant is not an array or not convertible to `T`.
    template <typename T>
    std::vector<T> getArrayCopy() const {
        assertIsCopyableOrConvertible<T>();
        return getArrayCopyImpl<T>();
    }

    /// Assign scalar value to variant (no copy).
    template <typename T>
    void setScalar(T& value) noexcept {
        assertIsNative<T>();
        setScalar(value, opcua::getDataType<T>());
    }

    /// Assign scalar value to variant with custom data type (no copy).
    template <typename T>
    void setScalar(T& value, const UA_DataType& dataType) noexcept {
        setScalarImpl(&value, dataType, UA_VARIANT_DATA_NODELETE);
    }

    /// Copy scalar value to variant.
    template <typename T>
    void setScalarCopy(const T& value) {
        assertIsCopyableOrConvertible<T>();
        if constexpr (detail::isRegisteredType<T>) {
            setScalarCopyImpl(value, opcua::getDataType<T>());
        } else {
            setScalarCopyConvertImpl(value);
        }
    }

    /// Copy scalar value to variant with custom data type.
    template <typename T>
    void setScalarCopy(const T& value, const UA_DataType& dataType) {
        setScalarCopyImpl(value, dataType);
    }

    /**
     * Assign array to variant (no copy).
     * @param array Container with a contiguous sequence of elements.
     *              For example `std::array`, `std::vector` or `Span`.
     *              The underlying array must be accessible with `std::data` and `std::size`.
     */
    template <typename ArrayLike>
    void setArray(ArrayLike&& array) noexcept {
        using ValueType = typename std::remove_reference_t<ArrayLike>::value_type;
        assertIsNative<ValueType>();
        setArray(std::forward<ArrayLike>(array), opcua::getDataType<ValueType>());
    }

    /**
     * Assign array to variant with custom data type (no copy).
     * @copydetails setArray
     * @param dataType Custom data type.
     */
    template <typename ArrayLike>
    void setArray(ArrayLike&& array, const UA_DataType& dataType) noexcept {
        static_assert(!isTemporaryArray<decltype(array)>());
        setArrayImpl(
            std::data(std::forward<ArrayLike>(array)),
            std::size(std::forward<ArrayLike>(array)),
            dataType,
            UA_VARIANT_DATA_NODELETE
        );
    }

    /**
     * Copy array to variant.
     * @param array Iterable container, for example `std::vector`, `std::list` or `Span`.
     *              The container must implement `begin()` and `end()`.
     */
    template <typename ArrayLike>
    void setArrayCopy(const ArrayLike& array) {
        setArrayCopy(array.begin(), array.end());
    }

    /**
     * Copy array to variant with custom data type.
     * @copydetails setArrayCopy
     * @param dataType Custom data type.
     */
    template <typename ArrayLike>
    void setArrayCopy(const ArrayLike& array, const UA_DataType& dataType) {
        setArrayCopy(array.begin(), array.end(), dataType);
    }

    /**
     * Copy range of elements as array to variant.
     */
    template <typename InputIt>
    void setArrayCopy(InputIt first, InputIt last) {
        using ValueType = typename std::iterator_traits<InputIt>::value_type;
        assertIsCopyableOrConvertible<ValueType>();
        if constexpr (detail::isRegisteredType<ValueType>) {
            setArrayCopyImpl(first, last, opcua::getDataType<ValueType>());
        } else {
            setArrayCopyConvertImpl(first, last);
        }
    }

    /**
     * Copy range of elements as array to variant with custom data type.
     */
    template <typename InputIt>
    void setArrayCopy(InputIt first, InputIt last, const UA_DataType& dataType) {
        setArrayCopyImpl(first, last, dataType);
    }

private:
    template <typename ArrayLike>
    static constexpr bool isTemporaryArray() {
        constexpr bool isTemporary = std::is_rvalue_reference_v<ArrayLike>;
        constexpr bool isView = detail::IsSpan<std::remove_reference_t<ArrayLike>>::value;
        return isTemporary && !isView;
    }

    template <typename T>
    static constexpr void assertIsNative() {
        static_assert(
            detail::isRegisteredType<T>,
            "Template type must be a native/wrapper type to assign or get scalar/array without copy"
        );
    }

    template <typename T>
    static constexpr void assertIsCopyableOrConvertible() {
        static_assert(
            detail::isRegisteredType<T> || detail::isConvertibleType<T>,
            "Template type must be either a native/wrapper type (copyable) or a convertible type. "
            "If the type is a native type: Provide the data type (UA_DataType) manually "
            "or register the type with a TypeRegistry template specialization. "
            "If the type should be converted: Add a template specialization for TypeConverter."
        );
    }

    template <typename T>
    static constexpr void assertNoVariant() {
        static_assert(
            !std::is_same_v<T, Variant> && !std::is_same_v<T, UA_Variant>,
            "Variants cannot directly contain another variant"
        );
    }

    void checkIsScalar() const {
        if (!isScalar()) {
            throw BadVariantAccess("Variant is not a scalar");
        }
    }

    void checkIsArray() const {
        if (!isArray()) {
            throw BadVariantAccess("Variant is not an array");
        }
    }

    template <typename T>
    void checkIsDataType() const {
        const auto* dt = getDataType();
        if (dt == nullptr || dt->typeId != opcua::getDataType<T>().typeId) {
            throw BadVariantAccess("Variant does not contain a value convertible to template type");
        }
    }

    template <typename T>
    inline T getScalarCopyImpl() const;
    template <typename T>
    inline std::vector<T> getArrayCopyImpl() const;

    template <typename T>
    inline void setScalarImpl(
        T* data, const UA_DataType& dataType, UA_VariantStorageType storageType
    ) noexcept;
    template <typename T>
    inline void setArrayImpl(
        T* data, size_t arrayLength, const UA_DataType& dataType, UA_VariantStorageType storageType
    ) noexcept;
    template <typename T>
    inline void setScalarCopyImpl(const T& value, const UA_DataType& dataType);
    template <typename T>
    inline void setScalarCopyConvertImpl(const T& value);
    template <typename InputIt>
    inline void setArrayCopyImpl(InputIt first, InputIt last, const UA_DataType& dataType);
    template <typename InputIt>
    inline void setArrayCopyConvertImpl(InputIt first, InputIt last);
};

template <typename T>
T Variant::getScalarCopyImpl() const {
    if constexpr (detail::isRegisteredType<T>) {
        return detail::copy(getScalar<T>(), opcua::getDataType<T>());
    } else {
        using Native = typename TypeConverter<T>::NativeType;
        T result{};
        TypeConverter<T>::fromNative(getScalar<Native>(), result);
        return result;
    }
}

template <typename T>
std::vector<T> Variant::getArrayCopyImpl() const {
    std::vector<T> result(handle()->arrayLength);
    if constexpr (detail::isRegisteredType<T>) {
        auto native = getArray<T>();
        std::transform(native.begin(), native.end(), result.begin(), [](auto&& value) {
            return detail::copy(value, opcua::getDataType<T>());
        });
    } else {
        using Native = typename TypeConverter<T>::NativeType;
        auto native = getArray<Native>();
        for (size_t i = 0; i < native.size(); ++i) {
            TypeConverter<T>::fromNative(native[i], result[i]);
        }
    }
    return result;
}

template <typename T>
void Variant::setScalarImpl(
    T* data, const UA_DataType& dataType, UA_VariantStorageType storageType
) noexcept {
    assertNoVariant<T>();
    assert(sizeof(T) == dataType.memSize);
    clear();
    handle()->type = &dataType;
    handle()->storageType = storageType;
    handle()->data = data;
}

template <typename T>
void Variant::setArrayImpl(
    T* data, size_t arrayLength, const UA_DataType& dataType, UA_VariantStorageType storageType
) noexcept {
    assertNoVariant<T>();
    assert(sizeof(T) == dataType.memSize);
    clear();
    handle()->type = &dataType;
    handle()->storageType = storageType;
    handle()->data = data;
    handle()->arrayLength = arrayLength;
}

template <typename T>
void Variant::setScalarCopyImpl(const T& value, const UA_DataType& dataType) {
    auto native = detail::allocateUniquePtr<T>(dataType);
    *native = detail::copy(value, dataType);
    setScalarImpl(native.release(), dataType, UA_VARIANT_DATA);  // move ownership
}

template <typename T>
void Variant::setScalarCopyConvertImpl(const T& value) {
    using Native = typename TypeConverter<T>::NativeType;
    const auto& dataType = opcua::getDataType<Native>();
    auto native = detail::allocateUniquePtr<Native>(dataType);
    TypeConverter<T>::toNative(value, *native);
    setScalarImpl(native.release(), dataType, UA_VARIANT_DATA);  // move ownership
}

template <typename InputIt>
void Variant::setArrayCopyImpl(InputIt first, InputIt last, const UA_DataType& dataType) {
    using ValueType = typename std::iterator_traits<InputIt>::value_type;
    const size_t size = std::distance(first, last);
    auto native = detail::allocateArrayUniquePtr<ValueType>(size, dataType);
    std::transform(first, last, native.get(), [&](const ValueType& value) {
        return detail::copy(value, dataType);
    });
    setArrayImpl(native.release(), size, dataType, UA_VARIANT_DATA);  // move ownership
}

template <typename InputIt>
void Variant::setArrayCopyConvertImpl(InputIt first, InputIt last) {
    using ValueType = typename std::iterator_traits<InputIt>::value_type;
    using Native = typename TypeConverter<ValueType>::NativeType;
    const auto& dataType = opcua::getDataType<Native>();
    const size_t size = std::distance(first, last);
    auto native = detail::allocateArrayUniquePtr<Native>(size, dataType);
    for (size_t i = 0; i < size; ++i) {
        TypeConverter<ValueType>::toNative(*first++, native.get()[i]);  // NOLINT
    }
    setArrayImpl(native.release(), size, dataType, UA_VARIANT_DATA);  // move ownership
}

namespace detail {

template <>
struct VariantHandler<VariantPolicy::Copy> {
    template <typename T>
    static void setScalar(Variant& var, const T& value) {
        var.setScalarCopy(value);
    }

    template <typename T>
    static void setScalar(Variant& var, const T& value, const UA_DataType& dtype) {
        var.setScalarCopy(value, dtype);
    }

    template <typename T>
    static void setArray(Variant& var, Span<T> array) {
        var.setArrayCopy(array.begin(), array.end());
    }

    template <typename T>
    static void setArray(Variant& var, Span<T> array, const UA_DataType& dtype) {
        var.setArrayCopy(array.begin(), array.end(), dtype);
    }

    template <typename InputIt>
    static void setArray(Variant& var, InputIt first, InputIt last) {
        var.setArrayCopy(first, last);
    }

    template <typename InputIt>
    static void setArray(Variant& var, InputIt first, InputIt last, const UA_DataType& dtype) {
        var.setArrayCopy(first, last, dtype);
    }
};

template <>
struct VariantHandler<VariantPolicy::Reference> {
    template <typename T>
    static void setScalar(Variant& var, T& value) noexcept {
        var.setScalar(value);
    }

    template <typename T>
    static void setScalar(Variant& var, T& value, const UA_DataType& dtype) noexcept {
        var.setScalar(value, dtype);
    }

    template <typename T>
    static void setArray(Variant& var, Span<T> array) noexcept {
        var.setArray(array);
    }

    template <typename T>
    static void setArray(Variant& var, Span<T> array, const UA_DataType& dtype) noexcept {
        var.setArray(array, dtype);
    }
};

template <>
struct VariantHandler<VariantPolicy::ReferenceIfPossible> : VariantHandler<VariantPolicy::Copy> {
    using VariantHandler<VariantPolicy::Copy>::setScalar;
    using VariantHandler<VariantPolicy::Copy>::setArray;

    template <typename T>
    static void setScalar(Variant& var, T& value) noexcept(detail::isRegisteredType<T>) {
        if constexpr (detail::isRegisteredType<T>) {
            var.setScalar(value);
        } else {
            var.setScalarCopy(value);
        }
    }

    template <typename T>
    static void setScalar(Variant& var, T& value, const UA_DataType& dtype) noexcept {
        var.setScalar(value, dtype);
    }

    template <typename T>
    static void setArray(Variant& var, Span<T> array) noexcept(detail::isRegisteredType<T>) {
        if constexpr (detail::isRegisteredType<T>) {
            var.setArray(array);
        } else {
            var.setArrayCopy(array);
        }
    }

    template <typename T>
    static void setArray(Variant& var, Span<T> array, const UA_DataType& dtype) noexcept {
        var.setArray(array, dtype);
    }

    template <typename T>
    static void setArray(Variant& var, Span<const T> array) {
        var.setArrayCopy(array.begin(), array.end());
    }

    template <typename T>
    static void setArray(Variant& var, Span<const T> array, const UA_DataType& dtype) {
        var.setArrayCopy(array.begin(), array.end(), dtype);
    }
};

}  // namespace detail

/* ------------------------------------------ DataValue ----------------------------------------- */

/**
 * UA_DataValue wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.11
 * @ingroup Wrapper
 */
class DataValue : public TypeWrapper<UA_DataValue, UA_TYPES_DATAVALUE> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    explicit DataValue(Variant value) noexcept {
        setValue(std::move(value));
    }

    DataValue(
        Variant value,
        std::optional<DateTime> sourceTimestamp,  // NOLINT
        std::optional<DateTime> serverTimestamp,  // NOLINT
        std::optional<uint16_t> sourcePicoseconds,
        std::optional<uint16_t> serverPicoseconds,
        std::optional<StatusCode> status
    ) noexcept
        : DataValue(UA_DataValue{
              UA_Variant{},
              sourceTimestamp.value_or(UA_DateTime{}),
              serverTimestamp.value_or(UA_DateTime{}),
              sourcePicoseconds.value_or(uint16_t{}),
              serverPicoseconds.value_or(uint16_t{}),
              status.value_or(UA_StatusCode{}),
              false,
              sourceTimestamp.has_value(),
              serverTimestamp.has_value(),
              sourcePicoseconds.has_value(),
              serverPicoseconds.has_value(),
              status.has_value(),
          }) {
        setValue(std::move(value));
    }

    /// Create DataValue from scalar value.
    /// @see Variant::fromScalar
    template <VariantPolicy Policy = VariantPolicy::Copy, typename... Args>
    [[nodiscard]] static DataValue fromScalar(Args&&... args) {
        return DataValue(Variant::fromScalar<Policy>(std::forward<Args>(args)...));
    }

    /// Create DataValue from array.
    /// @see Variant::fromArray
    template <VariantPolicy Policy = VariantPolicy::Copy, typename... Args>
    [[nodiscard]] static DataValue fromArray(Args&&... args) {
        return DataValue(Variant::fromArray<Policy>(std::forward<Args>(args)...));
    }

    bool hasValue() const noexcept {
        return handle()->hasValue;
    }

    bool hasSourceTimestamp() const noexcept {
        return handle()->hasSourceTimestamp;
    }

    bool hasServerTimestamp() const noexcept {
        return handle()->hasServerTimestamp;
    }

    bool hasSourcePicoseconds() const noexcept {
        return handle()->hasSourcePicoseconds;
    }

    bool hasServerPicoseconds() const noexcept {
        return handle()->hasServerPicoseconds;
    }

    bool hasStatus() const noexcept {
        return handle()->hasStatus;
    }

    /// Get value.
    Variant& value() & noexcept {
        return asWrapper<Variant>(handle()->value);
    }

    /// Get value.
    const Variant& value() const& noexcept {
        return asWrapper<Variant>(handle()->value);
    }

    /// Get value (rvalue).
    Variant&& value() && noexcept {
        return std::move(value());
    }

    /// Get value (rvalue).
    const Variant&& value() const&& noexcept {
        return std::move(value());  // NOLINT(*move-const-arg)
    }

    /// @deprecated Use value() instead
    [[deprecated("use value() instead")]]
    Variant& getValue() & noexcept {
        return value();
    }

    /// @deprecated Use value() instead
    [[deprecated("use value() instead")]]
    const Variant& getValue() const& noexcept {
        return value();
    }

    /// @deprecated Use value() instead
    [[deprecated("use value() instead")]]
    Variant&& getValue() && noexcept {
        return std::move(value());
    }

    /// @deprecated Use value() instead
    [[deprecated("use value() instead")]]
    const Variant&& getValue() const&& noexcept {
        return std::move(value());  // NOLINT(*move-const-arg)
    }

    /// Get source timestamp for the value.
    DateTime sourceTimestamp() const noexcept {
        return DateTime(handle()->sourceTimestamp);  // NOLINT
    }

    /// @deprecated Use sourceTimestamp() instead
    [[deprecated("use sourceTimestamp() instead")]]
    DateTime getSourceTimestamp() const noexcept {
        return sourceTimestamp();
    }

    /// Get server timestamp for the value.
    DateTime serverTimestamp() const noexcept {
        return DateTime(handle()->serverTimestamp);  // NOLINT
    }

    /// @deprecated Use serverTimestamp() instead
    [[deprecated("use serverTimestamp() instead")]]
    DateTime getServerTimestamp() const noexcept {
        return serverTimestamp();
    }

    /// Get picoseconds interval added to the source timestamp.
    uint16_t sourcePicoseconds() const noexcept {
        return handle()->sourcePicoseconds;
    }

    /// @deprecated Use sourcePicoseconds() instead
    [[deprecated("use sourcePicoseconds() instead")]]
    uint16_t getSourcePicoseconds() const noexcept {
        return sourcePicoseconds();
    }

    /// Get picoseconds interval added to the server timestamp.
    uint16_t serverPicoseconds() const noexcept {
        return handle()->serverPicoseconds;
    }

    /// @deprecated Use serverPicoseconds() instead
    [[deprecated("use serverPicoseconds() instead")]]
    uint16_t getServerPicoseconds() const noexcept {
        return serverPicoseconds();
    }

    /// Get status.
    StatusCode status() const noexcept {
        return handle()->status;
    }

    /// @deprecated Use status() instead
    [[deprecated("use status() instead")]]
    StatusCode getStatus() const noexcept {
        return status();
    }

    /// Set value (copy).
    void setValue(const Variant& value) {
        asWrapper<Variant>(handle()->value) = value;
        handle()->hasValue = true;
    }

    /// Set value (move).
    void setValue(Variant&& value) noexcept {
        asWrapper<Variant>(handle()->value) = std::move(value);
        handle()->hasValue = true;
    }

    /// Set source timestamp for the value.
    void setSourceTimestamp(DateTime sourceTimestamp) noexcept {  // NOLINT
        handle()->sourceTimestamp = sourceTimestamp.get();
        handle()->hasSourceTimestamp = true;
    }

    /// Set server timestamp for the value.
    void setServerTimestamp(DateTime serverTimestamp) noexcept {  // NOLINT
        handle()->serverTimestamp = serverTimestamp.get();
        handle()->hasServerTimestamp = true;
    }

    /// Set picoseconds interval added to the source timestamp.
    void setSourcePicoseconds(uint16_t sourcePicoseconds) noexcept {
        handle()->sourcePicoseconds = sourcePicoseconds;
        handle()->hasSourcePicoseconds = true;
    }

    /// Set picoseconds interval added to the server timestamp.
    void setServerPicoseconds(uint16_t serverPicoseconds) noexcept {
        handle()->serverPicoseconds = serverPicoseconds;
        handle()->hasServerPicoseconds = true;
    }

    /// Set status.
    void setStatus(StatusCode status) noexcept {
        handle()->status = status;
        handle()->hasStatus = true;
    }
};

/* --------------------------------------- ExtensionObject -------------------------------------- */

/**
 * Extension object encoding.
 * @see UA_ExtensionObjectEncoding
 */
enum class ExtensionObjectEncoding {
    // clang-format off
    EncodedNoBody     = UA_EXTENSIONOBJECT_ENCODED_NOBODY,
    EncodedByteString = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING,
    EncodedXml        = UA_EXTENSIONOBJECT_ENCODED_XML,
    Decoded           = UA_EXTENSIONOBJECT_DECODED,
    DecodedNoDelete   = UA_EXTENSIONOBJECT_DECODED_NODELETE
    // clang-format on
};

/**
 * UA_ExtensionObject wrapper class.
 *
 * ExtensionObjects may contain scalars of any data type. Even those that are unknown to the
 * receiver. If the received data type is unknown, the encoded string and target NodeId is stored
 * instead of the decoded data.
 *
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.1.6
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.2.2.15
 * @ingroup Wrapper
 */
class ExtensionObject : public TypeWrapper<UA_ExtensionObject, UA_TYPES_EXTENSIONOBJECT> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    /// Create an ExtensionObject from a decoded object (reference).
    /// The data will *not* be deleted when the ExtensionObject is destructed.
    /// @param data Decoded data
    template <typename T>
    [[nodiscard]] static ExtensionObject fromDecoded(T& data) noexcept {
        return fromDecoded(&data, getDataType<T>());
    }

    /// Create an ExtensionObject from a decoded object (reference).
    /// The data will *not* be deleted when the ExtensionObject is destructed.
    /// @param data Decoded data
    /// @param type Data type of the decoded data
    /// @warning Type erased version, use with caution.
    [[nodiscard]] static ExtensionObject fromDecoded(void* data, const UA_DataType& type) noexcept {
        ExtensionObject obj;
        obj->encoding = UA_EXTENSIONOBJECT_DECODED_NODELETE;
        obj->content.decoded.type = &type;  // NOLINT
        obj->content.decoded.data = data;  // NOLINT
        return obj;
    }

    /// Create an ExtensionObject from a decoded object (copy).
    /// Set the "decoded" data to a copy of the given object.
    /// @param data Decoded data
    template <typename T>
    [[nodiscard]] static ExtensionObject fromDecodedCopy(const T& data) {
        return fromDecodedCopy(&data, getDataType<T>());
    }

    /// Create an ExtensionObject from a decoded object (copy).
    /// @param data Decoded data
    /// @param type Data type of the decoded data
    /// @warning Type erased version, use with caution.
    [[nodiscard]] static ExtensionObject fromDecodedCopy(
        const void* data, const UA_DataType& type
    ) {
        // manual implementation instead of UA_ExtensionObject_setValueCopy to support open62541
        // v1.0 https://github.com/open62541/open62541/blob/v1.3.5/src/ua_types.c#L503-L524
        ExtensionObject obj;
        obj->encoding = UA_EXTENSIONOBJECT_DECODED;
        obj->content.decoded.data = detail::allocate<void>(type);  // NOLINT
        obj->content.decoded.type = &type;  // NOLINT
        throwIfBad(UA_copy(data, obj->content.decoded.data, &type));  // NOLINT
        return obj;
    }

    /// Check if the ExtensionObject is empty
    bool isEmpty() const noexcept {
        return (handle()->encoding == UA_EXTENSIONOBJECT_ENCODED_NOBODY);
    }

    /// Check if the ExtensionObject is encoded (usually if the data type is unknown).
    bool isEncoded() const noexcept {
        return (handle()->encoding == UA_EXTENSIONOBJECT_ENCODED_BYTESTRING) ||
            (handle()->encoding == UA_EXTENSIONOBJECT_ENCODED_XML);
    }

    /// Check if the ExtensionObject is decoded.
    bool isDecoded() const noexcept {
        return (handle()->encoding == UA_EXTENSIONOBJECT_DECODED) ||
            (handle()->encoding == UA_EXTENSIONOBJECT_DECODED_NODELETE);
    }

    /// Get the encoding.
    ExtensionObjectEncoding encoding() const noexcept {
        return static_cast<ExtensionObjectEncoding>(handle()->encoding);
    }

    /// @deprecated Use encoding() instead
    [[deprecated("use encoding() instead")]]
    ExtensionObjectEncoding getEncoding() const noexcept {
        return encoding();
    }

    /// Get the encoded type id.
    /// Returns `nullptr` if ExtensionObject is not encoded.
    const NodeId* encodedTypeId() const noexcept {
        return isEncoded()
            ? asWrapper<NodeId>(&handle()->content.encoded.typeId)  // NOLINT
            : nullptr;
    }

    /// @deprecated Use encodedTypeId() instead
    [[deprecated("use encodedTypeId() instead")]]
    const NodeId* getEncodedTypeId() const noexcept {
        return encodedTypeId();
    }

    /// Get the encoded body.
    /// Returns `nullptr` if ExtensionObject is not encoded.
    const ByteString* encodedBody() const noexcept {
        return isEncoded()
            ? asWrapper<ByteString>(&handle()->content.encoded.body)  // NOLINT
            : nullptr;
    }

    /// @deprecated Use encodedBody() instead
    [[deprecated("use encodedBody() instead")]]
    const ByteString* getEncodedBody() const noexcept {
        return encodedBody();
    }

    /// Get the decoded data type.
    /// Returns `nullptr` if ExtensionObject is not decoded.
    const UA_DataType* decodedType() const noexcept {
        return isDecoded()
            ? handle()->content.decoded.type  // NOLINT
            : nullptr;
    }

    /// @deprecated Use decodedType() instead
    [[deprecated("use decodedType() instead")]]
    const UA_DataType* getDecodedDataType() const noexcept {
        return decodedType();
    }

    /// Get pointer to the decoded data with given template type.
    /// Returns `nullptr` if the ExtensionObject is either not decoded or the decoded data is not of
    /// type `T`.
    template <typename T>
    T* decodedData() noexcept {
        return isDecodedType<T>() ? static_cast<T*>(decodedData()) : nullptr;
    }

    /// @deprecated Use decodedData<T>() instead
    template <typename T>
    [[deprecated("use decodedData<T>() instead")]]
    T* getDecodedData() noexcept {
        return decodedData<T>();
    }

    /// Get const pointer to the decoded data with given template type.
    /// Returns `nullptr` if the ExtensionObject is either not decoded or the decoded data is not of
    /// type `T`.
    template <typename T>
    const T* decodedData() const noexcept {
        return isDecodedType<T>() ? static_cast<const T*>(decodedData()) : nullptr;
    }

    /// @deprecated Use decodedData<T>() instead
    template <typename T>
    [[deprecated("use decodedData<T>() instead")]]
    const T* getDecodedData() const noexcept {
        return decodedData<T>();
    }

    /// Get pointer to the decoded data.
    /// Returns `nullptr` if the ExtensionObject is not decoded.
    /// @warning Type erased version, use with caution.
    void* decodedData() noexcept {
        return isDecoded()
            ? handle()->content.decoded.data  // NOLINT
            : nullptr;
    }

    /// @deprecated Use decodedData() instead
    [[deprecated("use decodedData() instead")]]
    void* getDecodedData() noexcept {
        return decodedData();
    }

    /// Get pointer to the decoded data.
    /// Returns `nullptr` if the ExtensionObject is not decoded.
    /// @warning Type erased version, use with caution.
    const void* decodedData() const noexcept {
        return isDecoded()
            ? handle()->content.decoded.data  // NOLINT
            : nullptr;
    }

    /// @deprecated Use decodedData() instead
    [[deprecated("use decodedData() instead")]]
    const void* getDecodedData() const noexcept {
        return decodedData();
    }

private:
    template <typename T>
    bool isDecodedType() const noexcept {
        const auto* type = decodedType();
        return (type != nullptr) && (type->typeId == getDataType<T>().typeId);
    }
};

/* --------------------------------------- DiagnosticInfo --------------------------------------- */

/**
 * UA_DiagnosticInfo wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.12
 * @ingroup Wrapper
 */
class DiagnosticInfo : public TypeWrapper<UA_DiagnosticInfo, UA_TYPES_DIAGNOSTICINFO> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    bool hasSymbolicId() const noexcept {
        return handle()->hasSymbolicId;
    }

    bool hasNamespaceUri() const noexcept {
        return handle()->hasNamespaceUri;
    }

    bool hasLocalizedText() const noexcept {
        return handle()->hasLocalizedText;
    }

    bool hasLocale() const noexcept {
        return handle()->hasLocale;
    }

    bool hasAdditionalInfo() const noexcept {
        return handle()->hasAdditionalInfo;
    }

    bool hasInnerStatusCode() const noexcept {
        return handle()->hasInnerStatusCode;
    }

    bool hasInnerDiagnosticInfo() const noexcept {
        return handle()->hasInnerDiagnosticInfo;
    }

    int32_t symbolicId() const noexcept {
        return handle()->symbolicId;
    }

    /// @deprecated Use symbolicId() instead
    [[deprecated("use symbolicId() instead")]]
    int32_t getSymbolicId() const noexcept {
        return symbolicId();
    }

    int32_t namespaceUri() const noexcept {
        return handle()->namespaceUri;
    }

    /// @deprecated Use namespaceUri() instead
    [[deprecated("use namespaceUri() instead")]]
    int32_t getNamespaceUri() const noexcept {
        return namespaceUri();
    }

    int32_t localizedText() const noexcept {
        return handle()->localizedText;
    }

    /// @deprecated Use localizedText() instead
    [[deprecated("use localizedText() instead")]]
    int32_t getLocalizedText() const noexcept {
        return localizedText();
    }

    int32_t locale() const noexcept {
        return handle()->locale;
    }

    /// @deprecated Use locale() instead
    [[deprecated("use locale() instead")]]
    int32_t getLocale() const noexcept {
        return locale();
    }

    const String& additionalInfo() const noexcept {
        return asWrapper<String>(handle()->additionalInfo);
    }

    /// @deprecated Use additionalInfo() instead
    [[deprecated("use additionalInfo() instead")]]
    const String& getAdditionalInfo() const noexcept {
        return additionalInfo();
    }

    StatusCode innerStatusCode() const noexcept {
        return handle()->innerStatusCode;
    }

    /// @deprecated Use innerStatusCode() instead
    [[deprecated("use innerStatusCode() instead")]]
    StatusCode getInnerStatusCode() const noexcept {
        return innerStatusCode();
    }

    const DiagnosticInfo* innerDiagnosticInfo() const noexcept {
        return asWrapper<DiagnosticInfo>(handle()->innerDiagnosticInfo);
    }

    /// @deprecated Use innerDiagnosticInfo() instead
    [[deprecated("use innerDiagnosticInfo() instead")]]
    const DiagnosticInfo* getInnerDiagnosticInfo() const noexcept {
        return innerDiagnosticInfo();
    }
};

/* ---------------------------------------- NumericRange ---------------------------------------- */

using NumericRangeDimension = UA_NumericRangeDimension;

inline bool operator==(
    const NumericRangeDimension& lhs, const NumericRangeDimension& rhs
) noexcept {
    return (lhs.min == rhs.min) && (lhs.max == rhs.max);
}

inline bool operator!=(
    const NumericRangeDimension& lhs, const NumericRangeDimension& rhs
) noexcept {
    return !(lhs == rhs);
}

/**
 * Numeric range to indicate subsets of (multidimensional) arrays.
 * They are no official data type in the OPC UA standard and are transmitted only with a string
 * encoding, such as "1:2,0:3,5". The colon separates min/max index and the comma separates
 * dimensions. A single value indicates a range with a single element (min==max).
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.27
 */
class NumericRange {
public:
    NumericRange() = default;

    explicit NumericRange(std::string_view encodedRange);

    explicit NumericRange(const char* encodedRange)  // required to avoid ambiguity
        : NumericRange(std::string_view(encodedRange)) {}

    explicit NumericRange(Span<const NumericRangeDimension> dimensions)
        : dimensions_(dimensions.begin(), dimensions.end()) {}

    explicit NumericRange(const UA_NumericRange& native)
        : NumericRange({native.dimensions, native.dimensionsSize}) {}

    bool empty() const noexcept {
        return dimensions_.empty();
    }

    Span<const NumericRangeDimension> dimensions() const noexcept {
        return dimensions_;
    }

    std::string toString() const;

private:
    std::vector<NumericRangeDimension> dimensions_;
};

}  // namespace opcua

/* ---------------------------------- std::hash specializations --------------------------------- */

template <>
struct std::hash<opcua::NodeId> {
    std::size_t operator()(const opcua::NodeId& id) const noexcept {
        return id.hash();
    }
};

template <>
struct std::hash<opcua::ExpandedNodeId> {
    std::size_t operator()(const opcua::ExpandedNodeId& id) const noexcept {
        return id.hash();
    }
};
