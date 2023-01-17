#include "open62541pp/TypeWrapper.h"

#include "open62541pp/Helper.h"

namespace opcua {

/* ------------------------------------------- String ------------------------------------------- */

String::String(std::string_view str)
    : String(UA_String{detail::allocUaString(str)}) {}

bool String::operator==(const String& other) const noexcept {
    return UA_String_equal(handle(), other.handle());
}

bool String::operator!=(const String& other) const noexcept {
    return !operator==(other);
}

std::string String::get() const {
    return detail::toString(*handle());
}

std::string_view String::getView() const {
    return detail::toStringView(*handle());
}

/* -------------------------------------------- Guid -------------------------------------------- */

Guid::Guid(UA_UInt32 data1, UA_UInt16 data2, UA_UInt16 data3, std::array<UA_Byte, 8> data4)
    : Guid(UA_Guid{
          data1,
          data2,
          data3,
          {data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]},
      }) {}

bool Guid::operator==(const Guid& other) const noexcept {
    return UA_Guid_equal(handle(), other.handle());
}

bool Guid::operator!=(const Guid& other) const noexcept {
    return !operator==(other);
}

/* ----------------------------------------- ByteString ----------------------------------------- */

ByteString::ByteString(std::string_view str)
    : ByteString(UA_ByteString{detail::allocUaString(str)}) {}

bool ByteString::operator==(const ByteString& other) const noexcept {
    return UA_ByteString_equal(handle(), other.handle());
}

bool ByteString::operator!=(const ByteString& other) const noexcept {
    return !operator==(other);
}

std::string ByteString::get() const {
    return detail::toString(*handle());
}

std::string_view ByteString::getView() const {
    return detail::toStringView(*handle());
}

/* ----------------------------------------- XmlElement ----------------------------------------- */

XmlElement::XmlElement(std::string_view str)
    : XmlElement(UA_XmlElement{detail::allocUaString(str)}) {}

bool XmlElement::operator==(const XmlElement& other) const noexcept {
    return UA_String_equal(handle(), other.handle());
}

bool XmlElement::operator!=(const XmlElement& other) const noexcept {
    return !operator==(other);
}

std::string XmlElement::get() const {
    return detail::toString(*handle());
}

std::string_view XmlElement::getView() const {
    return detail::toStringView(*handle());
}

/* ---------------------------------------- QualifiedName --------------------------------------- */

QualifiedName::QualifiedName(uint16_t namespaceIndex, std::string_view name)
    : QualifiedName(UA_QualifiedName{namespaceIndex, detail::allocUaString(name)}) {}

bool QualifiedName::operator==(const QualifiedName& other) const noexcept {
    return UA_QualifiedName_equal(handle(), other.handle());
}

bool QualifiedName::operator!=(const QualifiedName& other) const noexcept {
    return !operator==(other);
}

uint16_t QualifiedName::getNamespaceIndex() const noexcept {
    return handle()->namespaceIndex;
}

std::string QualifiedName::getName() const {
    return detail::toString(handle()->name);
}

std::string_view QualifiedName::getNameView() const {
    return detail::toStringView(handle()->name);
}

/* ---------------------------------------- LocalizedText --------------------------------------- */

LocalizedText::LocalizedText(std::string_view text, std::string_view locale)
    : LocalizedText(UA_LocalizedText{detail::allocUaString(locale), detail::allocUaString(text)}) {}

bool LocalizedText::operator==(const LocalizedText& other) const noexcept {
    return UA_String_equal(&handle()->text, &other.handle()->text) &&
           UA_String_equal(&handle()->locale, &other.handle()->locale);
}

bool LocalizedText::operator!=(const LocalizedText& other) const noexcept {
    return !operator==(other);
}

std::string LocalizedText::getText() const {
    return detail::toString(handle()->text);
}

std::string_view LocalizedText::getTextView() const {
    return detail::toStringView(handle()->text);
}

std::string LocalizedText::getLocale() const {
    return detail::toString(handle()->locale);
}

std::string_view LocalizedText::getLocaleView() const {
    return detail::toStringView(handle()->locale);
}

}  // namespace opcua
