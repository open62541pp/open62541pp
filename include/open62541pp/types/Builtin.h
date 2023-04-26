#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"

namespace opcua {

/**
 * UA_String wrapper class.
 * @ingroup TypeWrapper
 */
class String : public TypeWrapper<UA_String, UA_TYPES_STRING> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    explicit String(std::string_view str);

    bool empty() const noexcept;

    std::string_view get() const;
};

/**
 * UA_Guid wrapper class.
 * @ingroup TypeWrapper
 */
class Guid : public TypeWrapper<UA_Guid, UA_TYPES_GUID> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    Guid(uint32_t data1, uint16_t data2, uint16_t data3, std::array<uint8_t, 8> data4);

    static Guid random();

    std::string toString() const;
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

    bool empty() const noexcept;

    std::string_view get() const;
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

    bool empty() const noexcept;

    std::string_view get() const;
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
    std::string_view getName() const;
};

/**
 * UA_LocalizedText wrapper class.
 * The format of locale is `<language>[-<country/region>]`:
 * - `<language>` is the two letter ISO 639 code for a language
 * - `<country/region>` is the two letter ISO 3166 code for the country/region
 * @see https://reference.opcfoundation.org/Core/Part3/v104/docs/8.5/
 * @see https://reference.opcfoundation.org/Core/Part3/v104/docs/8.4/
 * @ingroup TypeWrapper
 */
class LocalizedText : public TypeWrapper<UA_LocalizedText, UA_TYPES_LOCALIZEDTEXT> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

    LocalizedText(std::string_view locale, std::string_view text, bool assertLocaleFormat = true);

    std::string_view getText() const;
    std::string_view getLocale() const;
};

}  // namespace opcua
