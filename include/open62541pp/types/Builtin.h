#pragma once

#include <algorithm>  // copy
#include <array>
#include <cstdint>
#include <iosfwd>  // forward declare ostream
#include <string>
#include <string_view>
#include <utility>  // move
#include <vector>

#include "open62541pp/Common.h"  // NamespaceIndex
#include "open62541pp/Config.h"  // UAPP_HAS_FILESYSTEM
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Wrapper.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/string_utils.h"

#ifdef UAPP_HAS_FILESYSTEM
#include <filesystem>
#endif

namespace opcua {

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

    constexpr StatusCode(UA_StatusCode code) noexcept  // NOLINT, implicit wanted
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

/**
 * UA_String wrapper class.
 * @ingroup Wrapper
 */
class String : public TypeWrapper<UA_String, UA_TYPES_STRING> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    explicit String(std::string_view str)
        : String(detail::allocNativeString(str)) {}

    /// Implicit conversion to std::string_view.
    operator std::string_view() const noexcept {  // NOLINT, implicit wanted
        return get();
    }

    bool empty() const noexcept {
        return handle()->length == 0U;
    }

    std::string_view get() const noexcept {
        return detail::toStringView(*handle());
    }
};

inline bool operator==(const UA_String& lhs, const UA_String& rhs) noexcept {
    return UA_String_equal(&lhs, &rhs);
}

inline bool operator!=(const UA_String& lhs, const UA_String& rhs) noexcept {
    return !(lhs == rhs);
}

inline bool operator==(const String& lhs, std::string_view rhs) noexcept {
    return (lhs.get() == rhs);
}

inline bool operator!=(const String& lhs, std::string_view rhs) noexcept {
    return (lhs.get() != rhs);
}

inline bool operator==(std::string_view lhs, const String& rhs) noexcept {
    return (lhs == rhs.get());
}

inline bool operator!=(std::string_view lhs, const String& rhs) noexcept {
    return (lhs != rhs.get());
}

std::ostream& operator<<(std::ostream& os, const String& string);

/**
 * UA_Guid wrapper class.
 * @ingroup Wrapper
 */
class Guid : public TypeWrapper<UA_Guid, UA_TYPES_GUID> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

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

/**
 * UA_ByteString wrapper class.
 * @ingroup Wrapper
 */
class ByteString : public TypeWrapper<UA_ByteString, UA_TYPES_BYTESTRING> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    explicit ByteString(std::string_view str)
        : ByteString(detail::allocNativeString(str)) {}

    explicit ByteString(const std::vector<uint8_t>& bytes) {
        throwIfBad(UA_ByteString_allocBuffer(handle(), bytes.size()));
        std::copy(bytes.begin(), bytes.end(), handle()->data);
    }

#ifdef UAPP_HAS_FILESYSTEM
    /// Read ByteString from binary file.
    static ByteString fromFile(const std::filesystem::path& filepath);
#endif

    /// Parse ByteString from Base64 encoded string.
    /// @note Supported since open62541 v1.1
    static ByteString fromBase64(std::string_view encoded);

    bool empty() const noexcept {
        return handle()->length == 0U;
    }

#ifdef UAPP_HAS_FILESYSTEM
    /// Write ByteString to binary file.
    void toFile(const std::filesystem::path& filepath) const;
#endif

    /// Convert to Base64 encoded string.
    /// @note Supported since open62541 v1.1
    std::string toBase64() const;

    std::string_view get() const noexcept {
        return detail::toStringView(*handle());
    }
};

inline bool operator==(const ByteString& lhs, std::string_view rhs) noexcept {
    return (lhs.get() == rhs);
}

inline bool operator!=(const ByteString& lhs, std::string_view rhs) noexcept {
    return (lhs.get() != rhs);
}

inline bool operator==(std::string_view lhs, const ByteString& rhs) noexcept {
    return (lhs == rhs.get());
}

inline bool operator!=(std::string_view lhs, const ByteString& rhs) noexcept {
    return (lhs != rhs.get());
}

/**
 * UA_XmlElement wrapper class.
 * @ingroup Wrapper
 */
class XmlElement : public TypeWrapper<UA_XmlElement, UA_TYPES_XMLELEMENT> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    explicit XmlElement(std::string_view str)
        : XmlElement(detail::allocNativeString(str)) {}

    bool empty() const noexcept {
        return handle()->length == 0U;
    }

    std::string_view get() const noexcept {
        return detail::toStringView(*handle());
    }
};

std::ostream& operator<<(std::ostream& os, const XmlElement& xmlElement);

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

    NamespaceIndex getNamespaceIndex() const noexcept {
        return handle()->namespaceIndex;
    }

    std::string_view getName() const noexcept {
        return detail::toStringView(handle()->name);
    }
};

inline bool operator==(const UA_QualifiedName& lhs, const UA_QualifiedName& rhs) noexcept {
    return (lhs.namespaceIndex == rhs.namespaceIndex) && (lhs.name == rhs.name);
}

inline bool operator!=(const UA_QualifiedName& lhs, const UA_QualifiedName& rhs) noexcept {
    return !(lhs == rhs);
}

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

    std::string_view getLocale() const noexcept {
        return detail::toStringView(handle()->locale);
    }

    std::string_view getText() const noexcept {
        return detail::toStringView(handle()->text);
    }
};

inline bool operator==(const UA_LocalizedText& lhs, const UA_LocalizedText& rhs) noexcept {
    return (lhs.locale == rhs.locale) && (lhs.text == rhs.text);
}

inline bool operator!=(const UA_LocalizedText& lhs, const UA_LocalizedText& rhs) noexcept {
    return !(lhs == rhs);
}

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

    int32_t getSymbolicId() const noexcept {
        return handle()->symbolicId;
    }

    int32_t getNamespaceUri() const noexcept {
        return handle()->namespaceUri;
    }

    int32_t getLocalizedText() const noexcept {
        return handle()->localizedText;
    }

    int32_t getLocale() const noexcept {
        return handle()->locale;
    }

    const String& getAdditionalInfo() const noexcept {
        return asWrapper<String>(handle()->additionalInfo);
    }

    StatusCode getInnerStatusCode() const noexcept {
        return handle()->innerStatusCode;
    }

    const DiagnosticInfo* getInnerDiagnosticInfo() const noexcept {
        return asWrapper<DiagnosticInfo>(handle()->innerDiagnosticInfo);
    }
};

/* ---------------------------------------------------------------------------------------------- */

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

    explicit NumericRange(std::vector<NumericRangeDimension> dimensions)
        : dimensions_(std::move(dimensions)) {}

    explicit NumericRange(const UA_NumericRange& native)
        : dimensions_(native.dimensions, native.dimensions + native.dimensionsSize) {}  // NOLINT

    bool empty() const noexcept {
        return dimensions_.empty();
    }

    const std::vector<NumericRangeDimension>& get() const noexcept {
        return dimensions_;
    }

    std::string toString() const;

private:
    std::vector<NumericRangeDimension> dimensions_;
};

}  // namespace opcua
