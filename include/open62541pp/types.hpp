#pragma once

#include <algorithm>  // copy
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>  // hash
#include <iosfwd>  // forward declare ostream
#include <iterator>  // reverse_iterator
#include <optional>
#include <ratio>
#include <string_view>
#include <type_traits>  // is_same_v
#include <utility>  // move
#include <vector>

#include "open62541pp/common.hpp"  // NamespaceIndex
#include "open62541pp/config.hpp"
#include "open62541pp/detail/iterator.hpp"  // TransformIterator
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/string_utils.hpp"  // allocNativeString
#include "open62541pp/detail/traits.hpp"
#include "open62541pp/detail/types_handling.hpp"
#include "open62541pp/exception.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/typeconverter.hpp"
#include "open62541pp/typeregistry.hpp"
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
        : Wrapper{code} {}

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

UAPP_TYPEREGISTRY_NATIVE(StatusCode, UA_TYPES_STATUSCODE)

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
        return stripEmptyArraySentinel(reinterpret_cast<pointer>(native.data));  // NOLINT
    }

    const_pointer data() const noexcept {
        const auto& native = asNative(static_cast<const WrapperType&>(*this));
        return stripEmptyArraySentinel(reinterpret_cast<const_pointer>(native.data));  // NOLINT
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
        native.data = allocateArray<uint8_t>(length);
        native.length = length;
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
    : public WrapperNative<UA_String, UA_TYPES_STRING>,
      public detail::StringLikeMixin<String, char> {
public:
    using Wrapper::Wrapper;

    explicit String(std::string_view str)
        : String{detail::allocNativeString(str)} {}

    template <typename InputIt>
    String(InputIt first, InputIt last) {
        init(first, last);
    }

    /// Assign null-termined character string.
    String& operator=(const char* str) {
        *this = String(str);
        return *this;
    }

    /// Assign std::string_view.
    template <typename Traits>
    String& operator=(std::basic_string_view<char, Traits> str) {
        *this = String(str);
        return *this;
    }

    /// Implicit conversion to std::string_view.
    template <typename Traits>
    operator std::basic_string_view<char, Traits>() const noexcept {  // NOLINT(*-conversions)
        return {data(), size()};
    }
};

/// @relates String
inline bool operator==(const UA_String& lhs, const UA_String& rhs) noexcept {
    return UA_String_equal(&lhs, &rhs);
}

/// @relates String
inline bool operator!=(const UA_String& lhs, const UA_String& rhs) noexcept {
    return !(lhs == rhs);
}

/// @relates String
inline bool operator==(const String& lhs, const String& rhs) noexcept {
    return asNative(lhs) == asNative(rhs);
}

/// @relates String
inline bool operator!=(const String& lhs, const String& rhs) noexcept {
    return !(lhs == rhs);
}

/// @relates String
inline bool operator==(const String& lhs, std::string_view rhs) noexcept {
    return static_cast<std::string_view>(lhs) == rhs;
}

/// @relates String
inline bool operator!=(const String& lhs, std::string_view rhs) noexcept {
    return static_cast<std::string_view>(lhs) != rhs;
}

/// @relates String
inline bool operator==(std::string_view lhs, const String& rhs) noexcept {
    return lhs == static_cast<std::string_view>(rhs);
}

/// @relates String
inline bool operator!=(std::string_view lhs, const String& rhs) noexcept {
    return lhs != static_cast<std::string_view>(rhs);
}

/// @relates String
std::ostream& operator<<(std::ostream& os, const String& str);

template <typename T>
struct TypeConverter<T, std::enable_if_t<detail::IsStringLike<T>::value>> {
    using NativeType = String;

    [[nodiscard]] static T fromNative(const String& src) {
        return T{src.data(), src.size()};
    }

    [[nodiscard]] static String toNative(const T& src) {
        return String{src.begin(), src.end()};
    }
};

template <>
struct TypeConverter<const char*> {
    using NativeType = String;

    [[nodiscard]] static String toNative(const char* src) {
        return String{src};
    }
};

template <size_t N>
struct TypeConverter<char[N]> {  // NOLINT(*c-arrays)
    using NativeType = String;

    [[nodiscard]] static String toNative(const char (&src)[N]) {  // NOLINT(*c-arrays)
        // string literals are null-terminated, trim null terminator if present
        const auto length = N > 0 && src[N - 1] == '\0' ? N - 1 : N;
        return String{std::string_view{static_cast<const char*>(src), length}};
    }
};

UAPP_TYPEREGISTRY_NATIVE(String, UA_TYPES_STRING)

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
class DateTime : public Wrapper<UA_DateTime> {
public:
    using DefaultClock = std::chrono::system_clock;
    using UaDuration = std::chrono::duration<int64_t, std::ratio<1, 10'000'000>>;

    using Wrapper::Wrapper;

    template <typename Clock, typename Duration>
    DateTime(std::chrono::time_point<Clock, Duration> timePoint)  // NOLINT(*-explicit-conversions)
        : DateTime{fromTimePoint(timePoint)} {}

    /// Get current DateTime.
    static DateTime now() noexcept {
        return DateTime{UA_DateTime_now()};  // NOLINT
    }

    /// Get DateTime from std::chrono::time_point.
    template <typename Clock, typename Duration>
    static DateTime fromTimePoint(std::chrono::time_point<Clock, Duration> timePoint) {
        return DateTime{
            int64_t{UA_DATETIME_UNIX_EPOCH} +
            std::chrono::duration_cast<UaDuration>(timePoint.time_since_epoch()).count()
        };
    }

    /// Get DateTime from Unix time.
    static DateTime fromUnixTime(int64_t unixTime) noexcept {
        return DateTime{UA_DateTime_fromUnixTime(unixTime)};  // NOLINT
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
    String format(std::string_view format, bool localtime = false) const;
};

template <typename Clock, typename Duration>
struct TypeConverter<std::chrono::time_point<Clock, Duration>> {
    using TimePoint = std::chrono::time_point<Clock, Duration>;
    using NativeType = DateTime;

    [[nodiscard]] static TimePoint fromNative(const DateTime& src) {
        return src.toTimePoint<Clock, Duration>();
    }

    [[nodiscard]] static DateTime toNative(const TimePoint& src) {
        return DateTime::fromTimePoint(src);
    }
};

UAPP_TYPEREGISTRY_NATIVE(DateTime, UA_TYPES_DATETIME)

/* -------------------------------------------- Guid -------------------------------------------- */

/**
 * UA_Guid wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.1.3
 * @ingroup Wrapper
 */
class Guid : public Wrapper<UA_Guid> {
public:
    using Wrapper::Wrapper;

    explicit Guid(std::array<uint8_t, 16> data) noexcept
        : Guid{UA_Guid{
              // NOLINTBEGIN(hicpp-signed-bitwise)
              static_cast<uint32_t>(
                  (data[0] << 24U) | (data[1] << 16U) | (data[2] << 8U) | data[3]
              ),
              static_cast<uint16_t>((data[4] << 8U) | data[5]),
              static_cast<uint16_t>((data[6] << 8U) | data[7]),
              // NOLINTEND(hicpp-signed-bitwise)
              {data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]},
          }} {}

    Guid(uint32_t data1, uint16_t data2, uint16_t data3, std::array<uint8_t, 8> data4) noexcept
        : Guid{UA_Guid{
              data1,
              data2,
              data3,
              {data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]},
          }} {}

    /// Generate random Guid.
    static Guid random() noexcept {
        return Guid(UA_Guid_random());  // NOLINT
    }

#if UAPP_HAS_PARSING
    /// Parse Guid from its string representation.
    /// Format: `C496578A-0DFE-4B8F-870A-745238C6AEAE`.
    static Guid parse(std::string_view str) {
        Guid guid;
        throwIfBad(UA_Guid_parse(guid.handle(), detail::toNativeString(str)));
        return guid;
    }
#endif

    /// @deprecated Use free function opcua::toString(const T&) instead
    [[deprecated("use free function toString instead")]]
    String toString() const;
};

/// @relates Guid
inline bool operator==(const UA_Guid& lhs, const UA_Guid& rhs) noexcept {
    return UA_Guid_equal(&lhs, &rhs);
}

/// @relates Guid
inline bool operator!=(const UA_Guid& lhs, const UA_Guid& rhs) noexcept {
    return !(lhs == rhs);
}

UAPP_TYPEREGISTRY_NATIVE(Guid, UA_TYPES_GUID)

/* ----------------------------------------- ByteString ----------------------------------------- */

/**
 * UA_ByteString wrapper class.
 * @ingroup Wrapper
 */
class ByteString
    : public WrapperNative<UA_ByteString, UA_TYPES_BYTESTRING>,
      public detail::StringLikeMixin<ByteString, uint8_t> {
public:
    using Wrapper::Wrapper;

    explicit ByteString(std::string_view str)
        : ByteString{detail::allocNativeString(str)} {}

    explicit ByteString(const char* str)  // required to avoid ambiguity
        : ByteString{std::string_view{str}} {}

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
    template <typename Traits>
    explicit operator std::basic_string_view<char, Traits>() const noexcept {
        return {reinterpret_cast<const char*>(data()), size()};  // NOLINT
    }

    /// Convert to Base64 encoded string.
    /// @note Supported since open62541 v1.1
    String toBase64() const;
};

UAPP_TYPEREGISTRY_NATIVE(ByteString, UA_TYPES_BYTESTRING)

/* ----------------------------------------- XmlElement ----------------------------------------- */

/**
 * UA_XmlElement wrapper class.
 * @ingroup Wrapper
 */
class XmlElement
    : public WrapperNative<UA_XmlElement, UA_TYPES_XMLELEMENT>,
      public detail::StringLikeMixin<XmlElement, char> {
public:
    using Wrapper::Wrapper;

    explicit XmlElement(std::string_view str)
        : XmlElement{detail::allocNativeString(str)} {}

    template <typename InputIt>
    XmlElement(InputIt first, InputIt last) {
        init(first, last);
    }

    /// Assign null-termined character string.
    XmlElement& operator=(const char* str) {
        *this = XmlElement(str);
        return *this;
    }

    /// Assign std::string_view.
    template <typename Traits>
    XmlElement& operator=(std::basic_string_view<char, Traits> str) {
        *this = XmlElement(str);
        return *this;
    }

    /// Implicit conversion to std::string_view.
    template <typename Traits>
    operator std::basic_string_view<char, Traits>() const noexcept {  // NOLINT(*-conversions)
        return {data(), size()};
    }
};

UAPP_TYPEREGISTRY_NATIVE(XmlElement, UA_TYPES_XMLELEMENT)

/* ------------------------------------------- NodeId ------------------------------------------- */

namespace detail {
template <typename T, typename = void>
struct IsNodeIdEnum : std::false_type {};

template <typename T>
struct IsNodeIdEnum<T, std::void_t<decltype(namespaceOf(std::declval<T>()))>> : std::is_enum<T> {};
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
class NodeId : public WrapperNative<UA_NodeId, UA_TYPES_NODEID> {
public:
    using Wrapper::Wrapper;

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
    /// The namespace is retrieved by calling e.g. `namespaceOf(opcua::ObjectId)`.
    /// Make sure to provide an overload for custom enum types.
    template <typename T, typename = std::enable_if_t<detail::IsNodeIdEnum<T>::value>>
    NodeId(T identifier) noexcept  // NOLINT(hicpp-explicit-conversions)
        : NodeId(namespaceOf(identifier).index, static_cast<uint32_t>(identifier)) {}

#if UAPP_HAS_PARSING
    /// Parse NodeId from its string representation.
    /// Format: `ns=<namespaceindex>;<type>=<value>`, e.g. `i=13` or `ns=10;s=HelloWorld`
    /// @see https://reference.opcfoundation.org/Core/Part6/v104/docs/5.3.1.10
    static NodeId parse(std::string_view str) {
        NodeId id;
        throwIfBad(UA_NodeId_parse(id.handle(), detail::toNativeString(str)));
        return id;
    }
#endif

    bool isNull() const noexcept {
        return UA_NodeId_isNull(handle());
    }

    uint32_t hash() const noexcept {
        return UA_NodeId_hash(handle());
    }

    NamespaceIndex namespaceIndex() const noexcept {
        return handle()->namespaceIndex;
    }

    NodeIdType identifierType() const noexcept {
        return static_cast<NodeIdType>(handle()->identifierType);
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
        } else if constexpr (std::is_same_v<T, String>) {
            return identifierType() == NodeIdType::String
                ? asWrapper<String>(&handle()->identifier.string)
                : nullptr;
        } else if constexpr (std::is_same_v<T, Guid>) {
            return identifierType() == NodeIdType::Guid
                ? asWrapper<Guid>(&handle()->identifier.guid)
                : nullptr;
        } else if constexpr (std::is_same_v<T, ByteString>) {
            return identifierType() == NodeIdType::ByteString
                ? asWrapper<ByteString>(&handle()->identifier.byteString)
                : nullptr;
        } else {
            return nullptr;
        }
        // NOLINTEND(cppcoreguidelines-pro-type-union-access)
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

    /// @deprecated Use free function opcua::toString(const T&) instead
    [[deprecated("use free function toString instead")]]
    String toString() const;
};

/// @relates NodeId
inline bool operator==(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return UA_NodeId_equal(&lhs, &rhs);
}

/// @relates NodeId
inline bool operator!=(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return !(lhs == rhs);
}

/// @relates NodeId
inline bool operator<(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return UA_NodeId_order(&lhs, &rhs) == UA_ORDER_LESS;
}

/// @relates NodeId
inline bool operator>(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return UA_NodeId_order(&lhs, &rhs) == UA_ORDER_MORE;
}

/// @relates NodeId
inline bool operator<=(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return (lhs < rhs) || (lhs == rhs);
}

/// @relates NodeId
inline bool operator>=(const UA_NodeId& lhs, const UA_NodeId& rhs) noexcept {
    return (lhs > rhs) || (lhs == rhs);
}

UAPP_TYPEREGISTRY_NATIVE(NodeId, UA_TYPES_NODEID)

/* --------------------------------------- ExpandedNodeId --------------------------------------- */

/**
 * UA_ExpandedNodeId wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.16
 * @ingroup Wrapper
 */
class ExpandedNodeId : public WrapperNative<UA_ExpandedNodeId, UA_TYPES_EXPANDEDNODEID> {
public:
    using Wrapper::Wrapper;

    explicit ExpandedNodeId(NodeId id) noexcept {
        asWrapper<NodeId>(handle()->nodeId) = std::move(id);
    }

    ExpandedNodeId(NodeId id, std::string_view namespaceUri, uint32_t serverIndex) {
        asWrapper<NodeId>(handle()->nodeId) = std::move(id);
        handle()->namespaceUri = detail::allocNativeString(namespaceUri);
        handle()->serverIndex = serverIndex;
    }

#if UAPP_HAS_PARSING
    /// Parse ExpandedNodeId from its string representation.
    /// Format: `svr=<serverindex>;ns=<namespaceindex>;<type>=<value>` or
    ///         `svr=<serverindex>;nsu=<uri>;<type>=<value>`
    /// @see https://reference.opcfoundation.org/Core/Part6/v104/docs/5.3.1.11
    static ExpandedNodeId parse(std::string_view str) {
        ExpandedNodeId id;
        throwIfBad(UA_ExpandedNodeId_parse(id.handle(), detail::toNativeString(str)));
        return id;
    }
#endif

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

    std::string_view namespaceUri() const {
        return detail::toStringView(handle()->namespaceUri);
    }

    uint32_t serverIndex() const noexcept {
        return handle()->serverIndex;
    }

    /// @deprecated Use free function opcua::toString(const T&) instead
    [[deprecated("use free function toString instead")]]
    String toString() const;
};

/// @relates ExpandedNodeId
inline bool operator==(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return UA_ExpandedNodeId_equal(&lhs, &rhs);
}

/// @relates ExpandedNodeId
inline bool operator!=(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return !(lhs == rhs);
}

/// @relates ExpandedNodeId
inline bool operator<(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return UA_ExpandedNodeId_order(&lhs, &rhs) == UA_ORDER_LESS;
}

/// @relates ExpandedNodeId
inline bool operator>(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return UA_ExpandedNodeId_order(&lhs, &rhs) == UA_ORDER_MORE;
}

/// @relates ExpandedNodeId
inline bool operator<=(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return (lhs < rhs) || (lhs == rhs);
}

/// @relates ExpandedNodeId
inline bool operator>=(const UA_ExpandedNodeId& lhs, const UA_ExpandedNodeId& rhs) noexcept {
    return (lhs > rhs) || (lhs == rhs);
}

UAPP_TYPEREGISTRY_NATIVE(ExpandedNodeId, UA_TYPES_EXPANDEDNODEID)

/* ---------------------------------------- QualifiedName --------------------------------------- */

/**
 * UA_QualifiedName wrapper class.
 * @ingroup Wrapper
 */
class QualifiedName : public WrapperNative<UA_QualifiedName, UA_TYPES_QUALIFIEDNAME> {
public:
    using Wrapper::Wrapper;

    QualifiedName(NamespaceIndex namespaceIndex, std::string_view name) {
        handle()->namespaceIndex = namespaceIndex;
        handle()->name = detail::allocNativeString(name);
    }

    NamespaceIndex namespaceIndex() const noexcept {
        return handle()->namespaceIndex;
    }

    std::string_view name() const noexcept {
        return detail::toStringView(handle()->name);
    }
};

/// @relates QualifiedName
inline bool operator==(const UA_QualifiedName& lhs, const UA_QualifiedName& rhs) noexcept {
    return (lhs.namespaceIndex == rhs.namespaceIndex) && (lhs.name == rhs.name);
}

/// @relates QualifiedName
inline bool operator!=(const UA_QualifiedName& lhs, const UA_QualifiedName& rhs) noexcept {
    return !(lhs == rhs);
}

UAPP_TYPEREGISTRY_NATIVE(QualifiedName, UA_TYPES_QUALIFIEDNAME)

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
class LocalizedText : public WrapperNative<UA_LocalizedText, UA_TYPES_LOCALIZEDTEXT> {
public:
    using Wrapper::Wrapper;

    LocalizedText(std::string_view locale, std::string_view text) {
        handle()->locale = detail::allocNativeString(locale);
        handle()->text = detail::allocNativeString(text);
    }

    std::string_view locale() const noexcept {
        return detail::toStringView(handle()->locale);
    }

    std::string_view text() const noexcept {
        return detail::toStringView(handle()->text);
    }
};

/// @relates LocalizedText
inline bool operator==(const UA_LocalizedText& lhs, const UA_LocalizedText& rhs) noexcept {
    return (lhs.locale == rhs.locale) && (lhs.text == rhs.text);
}

/// @relates LocalizedText
inline bool operator!=(const UA_LocalizedText& lhs, const UA_LocalizedText& rhs) noexcept {
    return !(lhs == rhs);
}

UAPP_TYPEREGISTRY_NATIVE(LocalizedText, UA_TYPES_LOCALIZEDTEXT)

/* ------------------------------------------- Variant ------------------------------------------ */

class BadVariantAccess : public TypeError {
public:
    using TypeError::TypeError;
};

/**
 * UA_Variant wrapper class.
 *
 * Variants may contain scalar values or arrays of any type together with a type definition.
 * The standard mandates that variants contain built-in data types only.
 * open62541 transparently handles this wrapping in the encoding layer. If the type is unknown to
 * the receiver, the variant stores the original ExtensionObject in binary or XML encoding.
 *
 * open62541pp enhances variant handling with the following features:
 *
 * 1. **Type category detection**: Identifies whether `T` is scalar or array:
 *    - **Scalar**, if `T` is:
 *      - A registered type (TypeRegistry spezialization). This applies to native types and
 *        @ref Wrapper "wrapper types".
 *      - A convertible type (TypeConverter spezialization).
 *    - **Array**, if `T` is a container type (e.g. `std::vector` or `std::list`) and does not
 *      satisfy the criterias of a scalar type.
 *
 *    Applied in @ref Variant::Variant "constructors", @ref assign, and @ref to functions:
 *
 *    @code
 *    opcua::Variant var{5};                  // set scalar (via constructor)
 *    auto value = var.to<int>();             // convert scalar
 *
 *    std::array<int> array{1, 2, 3};
 *    var.assign(array);                      // set array (via assign)
 *    auto vec = var.to<std::vector<int>>();  // convert array
 *    @endcode
 *
 * 2. **Type definition retrieval**: Automatically retrieves UA_DataType via TypeRegistry.
 *    For every registered type `T`, the type definition parameter can be omitted:
 *
 *    @code
 *    opcua::Variant var{5, UA_TYPES[UA_TYPES_INT]};  // explicit type definition
 *    opcua::Variant var{5};                          // auto-detected type definition
 *    @endcode
 *
 * 3. **Type conversion**: Convert non-native types using TypeConverter.
 *    Native `UA_*` types can be assigned and retrieved to/from variants without any conversion,
 *    because their binary layout is described by the type definition (UA_DataType). The same is
 *    true for wrapper types, that share the exact memory layout as their wrapped native type.
 *    Non-native types, like `std::string` from the STL, may not be describeable by UA_DataType
 *    because their memory layout is an implementation detail.
 *    Instead, the conversion between non-native and native types can be defined with template
 *    specializations of TypeConverter. If a type is convertible (TypeConverter specialization),
 *    the Variant automatically manages the conversion, requiring a copy:
 *
 *    @code
 *    opcua::Variant var{std::string{"test"}};     // convert to native type (copy)
 *    auto& native = var.scalar<opcua::String>();  // reference to native type (no copy)
 *    auto str = var.to<std::string>();            // conversion (copy required)
 *    @endcode
 *
 * @ingroup Wrapper
 */
class Variant : public WrapperNative<UA_Variant, UA_TYPES_VARIANT> {
private:
    template <typename T>
    static constexpr bool isVariant =
        std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, Variant> ||
        std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, UA_Variant>;

public:
    using Wrapper::Wrapper;

    /// Create Variant from a pointer to a scalar/array (no copy).
    /// @see assign(T*)
    template <typename T, typename = std::enable_if_t<!std::is_const_v<T>>>
    explicit Variant(T* ptr) noexcept {
        assign(ptr);
    }

    /// Create Variant from a pointer to a scalar/array with a custom data type (no copy).
    /// @see assign(T*, const UA_DataType&)
    template <typename T, typename = std::enable_if_t<!std::is_const_v<T>>>
    Variant(T* ptr, const UA_DataType& type) noexcept {
        assign(ptr, type);
    }

    /// Create Variant from a scalar/array (copy/move).
    /// @see assign(T&&)
    template <typename T, typename = std::enable_if_t<!isVariant<T>>>
    explicit Variant(T&& value) {
        assign(std::forward<T>(value));
    }

    /// Create Variant from a scalar/array with a custom data type (copy/move).
    /// @see assign(T&&, const UA_DataType&)
    template <typename T>
    Variant(T&& value, const UA_DataType& type) {
        assign(std::forward<T>(value), type);
    }

    /// Create Variant from a range of elements (copy).
    /// @see assign(InputIt, InputIt)
    template <typename InputIt>
    Variant(InputIt first, InputIt last) {
        assign(first, last);
    }

    /// Create Variant from a range of elements with a custom data type (copy).
    /// @see assign(InputIt, InputIt, const UA_DataType&)
    template <typename InputIt>
    Variant(InputIt first, InputIt last, const UA_DataType& type) {
        assign(first, last, type);
    }

    /**
     * @name Modifiers
     * Modify internal scalar/array value.
     * @{
     */

    void assign(std::nullptr_t ptr) noexcept = delete;
    void assign(std::nullptr_t ptr, const UA_DataType& type) noexcept = delete;

    /**
     * Assign pointer to scalar/array to variant (no copy).
     * The object will *not* be deleted when the Variant is destructed.
     * @param ptr Non-const pointer to a value to assign to the variant. This can be:
     *            - A pointer to a scalar native or wrapper value.
     *            - A pointer to a contiguous container such as `std::array` or `std::vector`
     *              holding native or wrapper elements.
     *              The underlying array must be accessible with `std::data` and `std::size`.
     *            - A `nullptr`, in which case the variant will be cleared.
     */
    template <typename T, typename = std::enable_if_t<!std::is_const_v<T>>>
    void assign(T* ptr) noexcept {
        if constexpr (isArrayType<T>()) {
            using ValueType = detail::RangeValueT<T>;
            assertIsRegistered<ValueType>();
            assign(ptr, opcua::getDataType<ValueType>());
        } else {
            assertIsRegistered<T>();
            assign(ptr, opcua::getDataType<T>());
        }
    }

    /**
     * Assign pointer to scalar/array to variant with custom data type (no copy).
     * @copydetails assign(T*)
     * @param type Custom data type.
     */
    template <typename T, typename = std::enable_if_t<!std::is_const_v<T>>>
    void assign(T* ptr, const UA_DataType& type) noexcept {
        if (ptr == nullptr) {
            clear();
        } else if constexpr (isArrayType<T>()) {
            setArrayImpl(std::data(*ptr), std::size(*ptr), type, UA_VARIANT_DATA_NODELETE);
        } else {
            setScalarImpl(ptr, type, UA_VARIANT_DATA_NODELETE);
        }
    }

    /**
     * Assign scalar/array to variant (copy/move and convert if required).
     * @param value Value to copy to the variant. It can be:
     *              - A scalar native, wrapper or convertible value.
     *              - A container with native, wrapper or convertible elements.
     *                The container must implement `begin()` and `end()`.
     */
    template <typename T>
    void assign(T&& value) {
        using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
        if constexpr (isArrayType<ValueType>()) {
            assign(std::begin(value), std::end(value));
        } else {
            assertIsRegisteredOrConvertible<ValueType>();
            if constexpr (IsRegistered<ValueType>::value) {
                setScalarCopyImpl(std::forward<T>(value), opcua::getDataType<ValueType>());
            } else {
                setScalarCopyConvertImpl(std::forward<T>(value));
            }
        }
    }

    /**
     * Assign scalar/array to variant with custom data type (copy/move).
     * @param value Value to copy to the variant. It can be:
     *              - A scalar native or wrapper value.
     *              - A container with native or wrapper elements.
     *                The container must implement `begin()` and `end()`.
     * @param type Custom data type.
     */
    template <typename T>
    void assign(T&& value, const UA_DataType& type) {
        using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
        if constexpr (isArrayType<ValueType>()) {
            setArrayCopyImpl(std::begin(value), std::end(value), type);
        } else {
            setScalarCopyImpl(std::forward<T>(value), type);
        }
    }

    /**
     * Assign range to variant (copy and convert if required).
     * @param first Iterator to the beginning of the range.
     * @param last Iterator to the end of the range.
     * @tparam InputIt Iterator of a container with native, wrapper or convertible elements.
     */
    template <typename InputIt>
    void assign(InputIt first, InputIt last) {
        using ValueType = typename std::iterator_traits<InputIt>::value_type;
        assertIsRegisteredOrConvertible<ValueType>();
        if constexpr (IsRegistered<ValueType>::value) {
            setArrayCopyImpl(first, last, opcua::getDataType<ValueType>());
        } else {
            setArrayCopyConvertImpl(first, last);
        }
    }

    /**
     * Assign range to variant with custom data type (copy).
     * @param first Iterator to the beginning of the range.
     * @param last Iterator to the end of the range.
     * @param type Custom data type.
     * @tparam InputIt Iterator of a container with native or wrapper elements.
     */
    template <typename InputIt>
    void assign(InputIt first, InputIt last, const UA_DataType& type) {
        setArrayCopyImpl(first, last, type);
    }

    /// Assign pointer to scalar/array to variant (no copy).
    /// @see assign(T*)
    template <typename T, typename = std::enable_if_t<!isVariant<T>>>
    Variant& operator=(T* value) noexcept {
        assign(value);
        return *this;
    }

    /// Assign scalar/array to variant (copy and convert if required).
    /// @see assign(T&&)
    template <typename T, typename = std::enable_if_t<!isVariant<T>>>
    Variant& operator=(T&& value) {
        assign(std::forward<T>(value));
        return *this;
    }

    /**
     * @name Observers
     * Check the type category, type definition and array structure of the internal value.
     */

    /// Check if the variant is empty.
    bool empty() const noexcept {
        return UA_Variant_isEmpty(handle());
    }

    /// Check if the variant is a scalar.
    bool isScalar() const noexcept {
        return UA_Variant_isScalar(handle());
    }

    /// Check if the variant is an array.
    bool isArray() const noexcept {
        return !empty() && !isScalar();
    }

    /// Check if the variant type is equal to the provided data type.
    bool isType(const UA_DataType* type) const noexcept {
        return type != nullptr && isType(asWrapper<NodeId>(type->typeId));
    }

    /// Check if the variant type is equal to the provided data type.
    bool isType(const UA_DataType& type) const noexcept {
        return isType(&type);
    }

    /// Check if the variant type is equal to the provided data type id.
    bool isType(const NodeId& id) const noexcept {
        return type() != nullptr && type()->typeId == id;
    }

    /// Check if the variant type is equal to the provided template type.
    template <typename T>
    bool isType() const noexcept {
        return isType(opcua::getDataType<T>());
    }

    /// Get data type.
    const UA_DataType* type() const noexcept {
        return handle()->type;
    }

    /// Get array length or 0 if variant is not an array.
    size_t arrayLength() const noexcept {
        return handle()->arrayLength;
    }

    /// Get array dimensions.
    Span<const uint32_t> arrayDimensions() const noexcept {
        return {handle()->arrayDimensions, handle()->arrayDimensionsSize};
    }

    /**
     * @name Accessors
     * Access and convert internal scalar/array value.
     * @{
     */

    /// Get pointer to the underlying data.
    /// Check the properties and data type before casting it to the actual type.
    /// Use the methods @ref isScalar, @ref isArray, @ref isType / @ref type.
    /// @note The @ref UA_EMPTY_ARRAY_SENTINEL sentinel, used to indicate an empty array, is
    ///       stripped from the returned pointer.
    void* data() noexcept {
        return detail::stripEmptyArraySentinel(handle()->data);
    }

    /// @copydoc data
    const void* data() const noexcept {
        return detail::stripEmptyArraySentinel(handle()->data);
    }

    /// Get scalar value with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not a scalar or not of type `T`.
    template <typename T>
    T& scalar() & {
        checkIsScalarType<T>();
        return *static_cast<T*>(data());
    }

    /// @copydoc scalar()&
    template <typename T>
    const T& scalar() const& {
        checkIsScalarType<T>();
        return *static_cast<const T*>(data());
    }

    /// @copydoc scalar()&
    template <typename T>
    [[nodiscard]] T scalar() && {
        return isBorrowed() ? scalar<T>() : std::move(scalar<T>());
    }

    /// @copydoc scalar()&
    template <typename T>
    [[nodiscard]] T scalar() const&& {
        return isBorrowed() ? scalar<T>() : std::move(scalar<T>());
    }

    /// Get array with given template type (only native or wrapper types).
    /// @exception BadVariantAccess If the variant is not an array or not of type `T`.
    template <typename T>
    Span<T> array() {
        checkIsArrayType<T>();
        return Span<T>(static_cast<T*>(data()), arrayLength());
    }

    /// @copydoc array()
    template <typename T>
    Span<const T> array() const {
        checkIsArrayType<T>();
        return Span<const T>(static_cast<const T*>(data()), arrayLength());
    }

    /**
     * Converts the variant to the specified type `T` with automatic conversion if required.
     *
     * Determines the type category (scalar or array) based on the characteristics of `T` using
     * @ref Variant "type category detection".
     * If `T` is a container, it must be constructible from an iterator pair.
     *
     * @code
     * // Scalar
     * opcua::Variant var{11};
     * const auto value = var.to<int>();
     * @endcode
     *
     * @code
     * // Array
     * std::array<std::string, 3> array{"One", "Two", "Three"};
     * opcua::Variant var{array};
     * const auto vec = var.to<std::vector<std::string>>();
     * const auto lst = var.to<std::list<opcua::String>>();
     * @endcode
     *
     * @exception BadVariantAccess If the variant is not convertible to `T`.
     */
    template <typename T>
    [[nodiscard]] T to() const& {
        return toImpl<T>(*this);
    }

    /// @copydoc to()const&
    template <typename T>
    [[nodiscard]] T to() && {
        return toImpl<T>(std::move(*this));
    }

    /**
     * @}
     */

private:
    template <typename T>
    static constexpr bool isScalarType() noexcept {
        return IsRegistered<T>::value || IsConvertible<T>::value;
    }

    template <typename T>
    static constexpr bool isArrayType() noexcept {
        return detail::IsRange<T>::value && !isScalarType<T>();
    }

    template <typename T>
    static constexpr void assertIsRegistered() {
        static_assert(
            IsRegistered<T>::value,
            "Template type must be a native/wrapper type to assign or get scalar/array without copy"
        );
    }

    template <typename T>
    static constexpr void assertIsRegisteredOrConvertible() {
        static_assert(
            IsRegistered<T>::value || IsConvertible<T>::value,
            "Template type must be either a native/wrapper type or a convertible type. "
            "If the type is a native type: Provide the type definition (UA_DataType) manually or "
            "register the type with a TypeRegistry template specialization. "
            "If the type should be converted: Add a template specialization for TypeConverter."
        );
    }

    template <typename T>
    static constexpr void assertNoVariant() {
        static_assert(!isVariant<T>, "Variants cannot directly contain another variant");
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
    void checkIsType() const {
        const auto* dt = type();
        if (dt == nullptr || dt->typeId != opcua::getDataType<T>().typeId) {
            throw BadVariantAccess("Variant does not contain a value convertible to template type");
        }
    }

    template <typename T>
    void checkIsScalarType() const {
        assertIsRegistered<T>();
        checkIsScalar();
        checkIsType<T>();
    }

    template <typename T>
    void checkIsArrayType() const {
        assertIsRegistered<T>();
        checkIsArray();
        checkIsType<T>();
    }

    bool isBorrowed() const noexcept {
        return handle()->storageType == UA_VARIANT_DATA_NODELETE;
    }

    template <typename T>
    void setScalarImpl(
        T* data, const UA_DataType& type, UA_VariantStorageType storageType
    ) noexcept {
        setArrayImpl(data, 0, type, storageType);
    }

    template <typename T>
    void setArrayImpl(
        T* data, size_t arrayLength, const UA_DataType& type, UA_VariantStorageType storageType
    ) noexcept {
        assertNoVariant<T>();
        assert(sizeof(T) == type.memSize);
        clear();
        handle()->type = &type;
        handle()->storageType = storageType;
        handle()->data = data;
        handle()->arrayLength = arrayLength;
    }

    template <typename T>
    void setScalarCopyImpl(T&& value, const UA_DataType& type) {
        using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
        auto native = detail::allocateUniquePtr<ValueType>(type);
        *native = detail::copy<ValueType>(std::forward<T>(value), type);
        setScalarImpl(native.release(), type, UA_VARIANT_DATA);  // move ownership
    }

    template <typename T>
    void setScalarCopyConvertImpl(T&& value) {
        using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
        using Native = typename TypeConverter<ValueType>::NativeType;
        const auto& type = opcua::getDataType<Native>();
        auto native = detail::allocateUniquePtr<Native>(type);
        *native = detail::toNative<ValueType>(std::forward<T>(value));
        setScalarImpl(native.release(), type, UA_VARIANT_DATA);  // move ownership
    }

    template <typename InputIt>
    void setArrayCopyImpl(InputIt first, InputIt last, const UA_DataType& type) {
        using ValueType = typename std::iterator_traits<InputIt>::value_type;
        const size_t size = std::distance(first, last);
        auto native = detail::allocateArrayUniquePtr<ValueType>(size, type);
        std::transform(first, last, native.get(), [&](auto&& value) {
            return detail::copy<ValueType>(std::forward<decltype(value)>(value), type);
        });
        setArrayImpl(native.release(), size, type, UA_VARIANT_DATA);  // move ownership
    }

    template <typename InputIt>
    void setArrayCopyConvertImpl(InputIt first, InputIt last) {
        using ValueType = typename std::iterator_traits<InputIt>::value_type;
        using Native = typename TypeConverter<ValueType>::NativeType;
        const auto& type = opcua::getDataType<Native>();
        const size_t size = std::distance(first, last);
        auto native = detail::allocateArrayUniquePtr<Native>(size, type);
        std::transform(first, last, native.get(), detail::toNative<ValueType>);
        setArrayImpl(native.release(), size, type, UA_VARIANT_DATA);  // move ownership
    }

    template <typename T, typename Self>
    static T toImpl(Self&& self) {
        if constexpr (isArrayType<T>()) {
            return toArrayImpl<T>(std::forward<Self>(self));
        } else {
            return toScalarImpl<T>(std::forward<Self>(self));
        }
    }

    template <typename T, typename Self>
    static T toScalarImpl(Self&& self) {
        assertIsRegisteredOrConvertible<T>();
        if constexpr (IsRegistered<T>::value) {
            return std::forward<Self>(self).template scalar<T>();
        } else {
            using Native = typename TypeConverter<T>::NativeType;
            return detail::fromNative<T>(std::forward<Self>(self).template scalar<Native>());
        }
    }

    template <typename T, typename Self>
    static T toArrayImpl(Self&& self) {
        using ValueType = detail::RangeValueT<T>;
        assertIsRegisteredOrConvertible<ValueType>();
        if constexpr (IsRegistered<ValueType>::value) {
            auto native = std::forward<Self>(self).template array<ValueType>();
            return T(native.begin(), native.end());
        } else {
            using Native = typename TypeConverter<ValueType>::NativeType;
            auto native = std::forward<Self>(self).template array<Native>();
            return T(
                detail::TransformIterator(native.begin(), detail::fromNative<ValueType>),
                detail::TransformIterator(native.end(), detail::fromNative<ValueType>)
            );
        }
    }
};

UAPP_TYPEREGISTRY_NATIVE(Variant, UA_TYPES_VARIANT)

/* ------------------------------------------ DataValue ----------------------------------------- */

/**
 * UA_DataValue wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.11
 * @ingroup Wrapper
 */
class DataValue : public WrapperNative<UA_DataValue, UA_TYPES_DATAVALUE> {
public:
    using Wrapper::Wrapper;

    explicit DataValue(Variant value) noexcept {
        setValue(std::move(value));
    }

    DataValue(
        Variant value,
        std::optional<DateTime> sourceTimestamp,  // NOLINT(*value-param)
        std::optional<DateTime> serverTimestamp,  // NOLINT(*value-param)
        std::optional<uint16_t> sourcePicoseconds,
        std::optional<uint16_t> serverPicoseconds,
        std::optional<StatusCode> status  // NOLINT(*value-param)
    ) noexcept
        : DataValue{UA_DataValue{
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
          }} {
        setValue(std::move(value));
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

    /// Get source timestamp for the value.
    DateTime sourceTimestamp() const noexcept {
        return DateTime{handle()->sourceTimestamp};  // NOLINT
    }

    /// Get server timestamp for the value.
    DateTime serverTimestamp() const noexcept {
        return DateTime{handle()->serverTimestamp};  // NOLINT
    }

    /// Get picoseconds interval added to the source timestamp.
    uint16_t sourcePicoseconds() const noexcept {
        return handle()->sourcePicoseconds;
    }

    /// Get picoseconds interval added to the server timestamp.
    uint16_t serverPicoseconds() const noexcept {
        return handle()->serverPicoseconds;
    }

    /// Get status.
    StatusCode status() const noexcept {
        return handle()->status;
    }
};

UAPP_TYPEREGISTRY_NATIVE(DataValue, UA_TYPES_DATAVALUE)

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
class ExtensionObject : public WrapperNative<UA_ExtensionObject, UA_TYPES_EXTENSIONOBJECT> {
private:
    template <typename T>
    static constexpr bool isExtensionObject =
        std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, ExtensionObject> ||
        std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, UA_ExtensionObject>;

public:
    using Wrapper::Wrapper;

    /**
     * Create ExtensionObject from a pointer to a decoded object (no copy).
     * The object will *not* be deleted when the ExtensionObject is destructed.
     * @param ptr Pointer to decoded object (native or wrapper).
     */
    template <typename T>
    explicit ExtensionObject(T* ptr) noexcept
        : ExtensionObject{ptr, getDataType<T>()} {}

    /**
     * Create ExtensionObject from a pointer to a decoded object with a custom data type (no copy).
     * The object will *not* be deleted when the ExtensionObject is destructed.
     * @param ptr Pointer to decoded object (native or wrapper).
     * @param type Data type of the decoded object.
     */
    template <typename T>
    ExtensionObject(T* ptr, const UA_DataType& type) noexcept {
        if (ptr == nullptr) {
            return;
        }
        assert(sizeof(T) == type.memSize);
        handle()->encoding = UA_EXTENSIONOBJECT_DECODED_NODELETE;
        handle()->content.decoded.type = &type;  // NOLINT
        handle()->content.decoded.data = ptr;  // NOLINT
    }

    /**
     * Create ExtensionObject from a decoded object (copy).
     * @param decoded Decoded object (native or wrapper).
     */
    template <typename T, typename = std::enable_if_t<!isExtensionObject<T>>>
    explicit ExtensionObject(const T& decoded)
        : ExtensionObject{decoded, getDataType<T>()} {}

    /**
     * Create ExtensionObject from a decoded object with a custom data type (copy).
     * @param decoded Decoded object (native or wrapper).
     * @param type Data type of the decoded object.
     */
    template <typename T, typename = std::enable_if_t<!isExtensionObject<T>>>
    explicit ExtensionObject(const T& decoded, const UA_DataType& type) {
        auto ptr = detail::allocateUniquePtr<T>(type);
        *ptr = detail::copy(decoded, type);
        handle()->encoding = UA_EXTENSIONOBJECT_DECODED;
        handle()->content.decoded.type = &type;  // NOLINT
        handle()->content.decoded.data = ptr.release();  // NOLINT
    }

    /// Check if the ExtensionObject is empty
    bool empty() const noexcept {
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

    /// Get the encoded type id.
    /// Returns `nullptr` if ExtensionObject is not encoded.
    const NodeId* encodedTypeId() const noexcept {
        return isEncoded()
            ? asWrapper<NodeId>(&handle()->content.encoded.typeId)  // NOLINT
            : nullptr;
    }

    /// Get the encoded body in binary format.
    /// Returns `nullptr` if ExtensionObject is not encoded in binary format.
    const ByteString* encodedBinary() const noexcept {
        return handle()->encoding == UA_EXTENSIONOBJECT_ENCODED_BYTESTRING
            ? asWrapper<ByteString>(&handle()->content.encoded.body)  // NOLINT
            : nullptr;
    }

    /// Get the encoded body in XML format.
    /// Returns `nullptr` if ExtensionObject is not encoded in XML format.
    const XmlElement* encodedXml() const noexcept {
        return handle()->encoding == UA_EXTENSIONOBJECT_ENCODED_XML
            ? asWrapper<XmlElement>(&handle()->content.encoded.body)  // NOLINT
            : nullptr;
    }

    /// Get the decoded data type.
    /// Returns `nullptr` if ExtensionObject is not decoded.
    const UA_DataType* decodedType() const noexcept {
        return isDecoded()
            ? handle()->content.decoded.type  // NOLINT
            : nullptr;
    }

    /// Get pointer to the decoded data with given template type.
    /// Returns `nullptr` if the ExtensionObject is either not decoded or the decoded data is not of
    /// type `T`.
    template <typename T>
    T* decodedData() noexcept {
        return isDecodedType<T>() ? static_cast<T*>(decodedData()) : nullptr;
    }

    /// Get const pointer to the decoded data with given template type.
    /// Returns `nullptr` if the ExtensionObject is either not decoded or the decoded data is not of
    /// type `T`.
    template <typename T>
    const T* decodedData() const noexcept {
        return isDecodedType<T>() ? static_cast<const T*>(decodedData()) : nullptr;
    }

    /// Get pointer to the decoded data.
    /// Returns `nullptr` if the ExtensionObject is not decoded.
    /// @warning Type erased version, use with caution.
    void* decodedData() noexcept {
        return isDecoded()
            ? handle()->content.decoded.data  // NOLINT
            : nullptr;
    }

    /// Get pointer to the decoded data.
    /// Returns `nullptr` if the ExtensionObject is not decoded.
    /// @warning Type erased version, use with caution.
    const void* decodedData() const noexcept {
        return isDecoded()
            ? handle()->content.decoded.data  // NOLINT
            : nullptr;
    }

private:
    template <typename T>
    bool isDecodedType() const noexcept {
        const auto* type = decodedType();
        return (type != nullptr) && (type->typeId == getDataType<T>().typeId);
    }
};

UAPP_TYPEREGISTRY_NATIVE(ExtensionObject, UA_TYPES_EXTENSIONOBJECT)

/* --------------------------------------- DiagnosticInfo --------------------------------------- */

/**
 * UA_DiagnosticInfo wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.12
 * @ingroup Wrapper
 */
class DiagnosticInfo : public WrapperNative<UA_DiagnosticInfo, UA_TYPES_DIAGNOSTICINFO> {
public:
    using Wrapper::Wrapper;

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

    int32_t namespaceUri() const noexcept {
        return handle()->namespaceUri;
    }

    int32_t localizedText() const noexcept {
        return handle()->localizedText;
    }

    int32_t locale() const noexcept {
        return handle()->locale;
    }

    const String& additionalInfo() const noexcept {
        return asWrapper<String>(handle()->additionalInfo);
    }

    StatusCode innerStatusCode() const noexcept {
        return handle()->innerStatusCode;
    }

    const DiagnosticInfo* innerDiagnosticInfo() const noexcept {
        return asWrapper<DiagnosticInfo>(handle()->innerDiagnosticInfo);
    }
};

UAPP_TYPEREGISTRY_NATIVE(DiagnosticInfo, UA_TYPES_DIAGNOSTICINFO)

/* ---------------------------------------- NumericRange ---------------------------------------- */

using NumericRangeDimension = UA_NumericRangeDimension;

/// @relates NumericRange
inline bool operator==(
    const NumericRangeDimension& lhs, const NumericRangeDimension& rhs
) noexcept {
    return (lhs.min == rhs.min) && (lhs.max == rhs.max);
}

/// @relates NumericRange
inline bool operator!=(
    const NumericRangeDimension& lhs, const NumericRangeDimension& rhs
) noexcept {
    return !(lhs == rhs);
}

/**
 * UA_NumericRange wrapper class.
 *
 * Numeric ranges indicate subsets of (multidimensional) arrays.
 * They are no official data type in the OPC UA standard and are transmitted only with a string
 * encoding, such as "1:2,0:3,5". The colon separates min/max index and the comma separates
 * dimensions. A single value indicates a range with a single element (min==max).
 *
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.27
 * @ingroup Wrapper
 */
class NumericRange : public Wrapper<UA_NumericRange> {
public:
    NumericRange() = default;

    /// Create a NumericRange from the encoded representation, e.g. `1:2,0:3,5`.
    explicit NumericRange(std::string_view encodedRange);

    /// @overload
    explicit NumericRange(const char* encodedRange)  // required to avoid ambiguity
        : NumericRange{std::string_view{encodedRange}} {}

    /// Create a NumericRange from dimensions.
    explicit NumericRange(Span<const NumericRangeDimension> dimensions)
        : Wrapper{copy(dimensions.data(), dimensions.size())} {}

    /// Create a NumericRange from native object (copy).
    explicit NumericRange(const UA_NumericRange& native)
        : Wrapper{copy(native)} {}

    /// Create a NumericRange from native object (move).
    NumericRange(UA_NumericRange&& native) noexcept  // NOLINT
        : Wrapper{std::exchange(native, {})} {}

    ~NumericRange() {
        clear();
    }

    NumericRange(const NumericRange& other)
        : Wrapper{copy(other.native())} {}

    NumericRange(NumericRange&& other) noexcept
        : Wrapper{std::exchange(other.native(), {})} {}

    NumericRange& operator=(const NumericRange& other) {
        if (this != &other) {
            clear();
            native() = copy(other.native());
        }
        return *this;
    }

    NumericRange& operator=(NumericRange&& other) noexcept {
        if (this != &other) {
            clear();
            native() = std::exchange(other.native(), {});
        }
        return *this;
    }

    bool empty() const noexcept {
        return native().dimensionsSize == 0;
    }

    Span<const NumericRangeDimension> dimensions() const noexcept {
        return {native().dimensions, native().dimensionsSize};
    }

    /// @deprecated Use free function opcua::toString(const T&) instead
    [[deprecated("use free function toString instead")]]
    String toString() const;

private:
    void clear() noexcept {
        detail::deallocateArray(native().dimensions);
        native() = {};
    }

    [[nodiscard]] static UA_NumericRange copy(const UA_NumericRangeDimension* array, size_t size) {
        UA_NumericRange result{};
        result.dimensions = detail::copyArray(array, size);
        result.dimensionsSize = size;
        return result;
    }

    [[nodiscard]] static UA_NumericRange copy(const UA_NumericRange& other) {
        return copy(other.dimensions, other.dimensionsSize);
    }
};

String toString(const NumericRange& range);

/* --------------------------------------- Free functions --------------------------------------- */

/**
 * Converts an object to its string representation.
 *
 * This function wraps @ref UA_print to generate a string representation of the given object, based
 * on its data type description.
 *
 * @note
 * Requires open62541 v1.2 or later.
 * The open62541 library must be compiled with the `UA_ENABLE_TYPEDESCRIPTION` option.
 * If these conditions are not met, the function returns an empty String.
 *
 * @param object Native or wrapper object.
 * @param type Data type of `object`.
 *
 * @relates TypeWrapper
 * @ingroup Wrapper
 */
template <typename T>
String toString(const T& object, const UA_DataType& type) {
    String output;
    if constexpr (UAPP_HAS_TOSTRING) {
        throwIfBad(UA_print(&object, &type, output.handle()));
    }
    return output;
}

/**
 * @overload
 * @relates TypeWrapper
 * @ingroup Wrapper
 */
template <typename T, typename = std::enable_if_t<IsRegistered<T>::value>>
String toString(const T& object) {
    return toString(object, getDataType<T>());
}

/* ------------------------------------ Comparison operators ------------------------------------ */

#if UAPP_HAS_ORDER

/// @relates TypeWrapper
/// @ingroup Wrapper
template <typename T, typename = std::enable_if_t<IsRegistered<T>::value>>
inline bool operator==(const T& lhs, const T& rhs) noexcept {
    return UA_order(&lhs, &rhs, &getDataType<T>()) == UA_ORDER_EQ;
}

/// @relates TypeWrapper
/// @ingroup Wrapper
template <typename T, typename = std::enable_if_t<IsRegistered<T>::value>>
inline bool operator!=(const T& lhs, const T& rhs) noexcept {
    return UA_order(&lhs, &rhs, &getDataType<T>()) != UA_ORDER_EQ;
}

/// @relates TypeWrapper
/// @ingroup Wrapper
template <typename T, typename = std::enable_if_t<IsRegistered<T>::value>>
inline bool operator<(const T& lhs, const T& rhs) noexcept {
    return UA_order(&lhs, &rhs, &getDataType<T>()) == UA_ORDER_LESS;
}

/// @relates TypeWrapper
/// @ingroup Wrapper
template <typename T, typename = std::enable_if_t<IsRegistered<T>::value>>
inline bool operator<=(const T& lhs, const T& rhs) noexcept {
    return UA_order(&lhs, &rhs, &getDataType<T>()) <= UA_ORDER_EQ;
}

/// @relates TypeWrapper
/// @ingroup Wrapper
template <typename T, typename = std::enable_if_t<IsRegistered<T>::value>>
inline bool operator>(const T& lhs, const T& rhs) noexcept {
    return UA_order(&lhs, &rhs, &getDataType<T>()) == UA_ORDER_MORE;
}

/// @relates TypeWrapper
/// @ingroup Wrapper
template <typename T, typename = std::enable_if_t<IsRegistered<T>::value>>
inline bool operator>=(const T& lhs, const T& rhs) noexcept {
    return UA_order(&lhs, &rhs, &getDataType<T>()) >= UA_ORDER_EQ;
}

#endif

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
