#include "open62541pp/types/Builtin.h"

#include <algorithm>  // copy
#include <cassert>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iterator>  // istreambuf_iterator
#include <ostream>
#include <sstream>
#include <utility>  // move

#include "open62541pp/Config.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/detail/helper.h"

namespace opcua {

/* ------------------------------------------- String ------------------------------------------- */

String::String(std::string_view str)
    : String(detail::allocNativeString(str)) {}

bool String::empty() const noexcept {
    return handle()->length == 0U;
}

std::string_view String::get() const noexcept {
    return detail::toStringView(*handle());
}

std::ostream& operator<<(std::ostream& os, const String& string) {
    os << string.get();
    return os;
}

/* -------------------------------------------- Guid -------------------------------------------- */

Guid::Guid(UA_UInt32 data1, UA_UInt16 data2, UA_UInt16 data3, std::array<UA_Byte, 8> data4) noexcept
    : Guid(UA_Guid{
          data1,
          data2,
          data3,
          {data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]},
      }) {}

Guid Guid::random() noexcept {
    return Guid(UA_Guid_random());  // NOLINT
}

std::string Guid::toString() const {
    // <Data1>-<Data2>-<Data3>-<Data4[0:1]>-<Data4[2:7]>
    // each value is formatted as a hexadecimal number with padded zeros
    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');

    ss << std::setw(8) << handle()->data1 << "-";
    ss << std::setw(4) << handle()->data2 << "-";
    ss << std::setw(4) << handle()->data3 << "-";

    const auto writeBit = [&](uint8_t bit) { ss << std::setw(2) << static_cast<int>(bit); };
    for (size_t i = 0; i <= 1; ++i) {
        writeBit(handle()->data4[i]);  // NOLINT
    }
    ss << "-";
    for (size_t i = 2; i <= 7; ++i) {
        writeBit(handle()->data4[i]);  // NOLINT
    }
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Guid& guid) {
    os << guid.toString();
    return os;
}

/* ----------------------------------------- ByteString ----------------------------------------- */

ByteString::ByteString(std::string_view str)
    : ByteString(detail::allocNativeString(str)) {}

ByteString::ByteString(const std::vector<uint8_t>& bytes) {
    const auto status = UA_ByteString_allocBuffer(handle(), bytes.size());
    throwIfBad(status);
    std::copy(bytes.begin(), bytes.end(), handle()->data);
}

bool ByteString::empty() const noexcept {
    return handle()->length == 0U;
}

std::string_view ByteString::get() const noexcept {
    return detail::toStringView(*handle());
}

ByteString ByteString::fromBase64([[maybe_unused]] std::string_view encoded) {
#if UAPP_OPEN62541_VER_GE(1, 1)
    ByteString output;
    UA_ByteString_fromBase64(output.handle(), String(encoded).handle());
    return output;
#else
    return {};
#endif
}

ByteString ByteString::fromFile(const fs::path& filepath) {
    std::ifstream fp(filepath, std::ios::binary);
    const std::vector<uint8_t> bytes(
        (std::istreambuf_iterator<char>(fp)), (std::istreambuf_iterator<char>())
    );
    return ByteString(bytes);
}

// NOLINTNEXTLINE
std::string ByteString::toBase64() const {
#if UAPP_OPEN62541_VER_GE(1, 1)
    String output;
    UA_ByteString_toBase64(handle(), output.handle());
    return std::string(output.get());
#else
    return {};
#endif
}

void ByteString::toFile(const fs::path& filepath) const {
    std::ofstream fp(filepath, std::ios::binary);
    fp.write(reinterpret_cast<char*>(handle()->data), handle()->length);  // NOLINT
}

/* ----------------------------------------- XmlElement ----------------------------------------- */

XmlElement::XmlElement(std::string_view str)
    : XmlElement(detail::allocNativeString(str)) {}

bool XmlElement::empty() const noexcept {
    return handle()->length == 0U;
}

std::string_view XmlElement::get() const noexcept {
    return detail::toStringView(*handle());
}

std::ostream& operator<<(std::ostream& os, const XmlElement& xmlElement) {
    os << xmlElement.get();
    return os;
}

/* ---------------------------------------- QualifiedName --------------------------------------- */

QualifiedName::QualifiedName(NamespaceIndex namespaceIndex, std::string_view name) {
    handle()->namespaceIndex = namespaceIndex;
    handle()->name = detail::allocNativeString(name);
}

NamespaceIndex QualifiedName::getNamespaceIndex() const noexcept {
    return handle()->namespaceIndex;
}

std::string_view QualifiedName::getName() const noexcept {
    return detail::toStringView(handle()->name);
}

/* ---------------------------------------- LocalizedText --------------------------------------- */

LocalizedText::LocalizedText(
    std::string_view locale, std::string_view text, bool assertLocaleFormat
) {
    handle()->locale = detail::allocNativeString(locale);
    handle()->text = detail::allocNativeString(text);
    if (assertLocaleFormat) {
        assert(
            (locale.empty() || locale.size() == 2 || locale.size() == 5) &&
            "locale must be of format <language>[-<country/region>]"
        );
    }
}

std::string_view LocalizedText::getText() const noexcept {
    return detail::toStringView(handle()->text);
}

std::string_view LocalizedText::getLocale() const noexcept {
    return detail::toStringView(handle()->locale);
}

/* ---------------------------------------- NumericRange ---------------------------------------- */

NumericRange::NumericRange(std::string_view encodedRange) {
    UA_String encodedRangeNative = detail::toNativeString(encodedRange);
    UA_NumericRange native{};
#if UAPP_OPEN62541_VER_GE(1, 1)
    const auto status = UA_NumericRange_parse(&native, encodedRangeNative);
#else
    const auto status = UA_NumericRange_parseFromString(&native, &encodedRangeNative);
#endif
    dimensions_.assign(
        native.dimensions,
        native.dimensions + native.dimensionsSize  // NOLINT
    );
    UA_free(native.dimensions);  // NOLINT
    throwIfBad(status);
}

NumericRange::NumericRange(std::vector<NumericRangeDimension> dimensions)
    : dimensions_(std::move(dimensions)) {}

NumericRange::NumericRange(const UA_NumericRange& native)
    : dimensions_(native.dimensions, native.dimensions + native.dimensionsSize) {}  // NOLINT

std::string NumericRange::toString() const {
    std::ostringstream ss;
    for (size_t i = 0; i < dimensions_.size(); ++i) {
        const auto& dimension = dimensions_.at(i);
        ss << dimension.min;
        if (dimension.min != dimension.max) {
            ss << ":" << dimension.max;
        }
        if (i < dimensions_.size() - 1) {
            ss << ",";
        }
    }
    return ss.str();
}

}  // namespace opcua
