#include "open62541pp/types.hpp"

#include <ctime>  // gmtime, localtime
#include <iomanip>  // put_time
#include <locale>
#include <ostream>
#include <sstream>

#include "open62541pp/config.hpp"

namespace opcua {

/* ------------------------------------------- String ------------------------------------------- */

std::ostream& operator<<(std::ostream& os, const String& str) {
    os << static_cast<std::string_view>(str);
    return os;
}

/* -------------------------------------------- Guid -------------------------------------------- */

String Guid::toString() const {
    return opcua::toString(*this);
}

/* ------------------------------------------ DateTime ------------------------------------------ */

String DateTime::format(std::string_view format, bool localtime) const {
    const std::time_t unixTime = toUnixTime();
    std::ostringstream ss;
    const auto* timeinfo = localtime ? std::localtime(&unixTime) : std::gmtime(&unixTime);
    if (timeinfo != nullptr) {
        ss << std::put_time(timeinfo, std::string{format}.c_str());
    }
    return String{ss.str()};
}

/* ----------------------------------------- ByteString ----------------------------------------- */

ByteString ByteString::fromBase64([[maybe_unused]] std::string_view encoded) {
#if UAPP_OPEN62541_VER_GE(1, 1)
    ByteString output;
    UA_ByteString_fromBase64(output.handle(), String{encoded}.handle());
    return output;
#else
    return {};
#endif
}

#if UAPP_OPEN62541_VER_GE(1, 1)
String ByteString::toBase64() const {
    String output;
    UA_ByteString_toBase64(handle(), output.handle());
    return output;
}
#endif

/* ------------------------------------------- NodeId ------------------------------------------- */

String NodeId::toString() const {
    return opcua::toString(*this);
}

#if UAPP_OPEN62541_VER_GE(1, 1)
String toString(const NodeId& id) {
    // UA_NodeId_print, unlike the generic UA_print, emits the standard textual NodeId format that
    // NodeId::parse accepts. UA_print wraps the same content in quotes, so its output round-trips
    // to BadDecodingError.
    String output;
    throwIfBad(UA_NodeId_print(id.handle(), output.handle()));
    return output;
}
#endif

/* --------------------------------------- ExpandedNodeId --------------------------------------- */

String ExpandedNodeId::toString() const {
    return opcua::toString(*this);
}

#if UAPP_OPEN62541_VER_GE(1, 2)
String toString(const ExpandedNodeId& id) {
    String output;
    throwIfBad(UA_ExpandedNodeId_print(id.handle(), output.handle()));
    return output;
}
#endif

/* ---------------------------------------- NumericRange ---------------------------------------- */

NumericRange::NumericRange(std::string_view encodedRange) {
    const auto encodedRangeNative = detail::toNativeString(encodedRange);
#if UAPP_OPEN62541_VER_GE(1, 1)
    throwIfBad(UA_NumericRange_parse(handle(), encodedRangeNative));
#else
    throwIfBad(UA_NumericRange_parseFromString(handle(), &encodedRangeNative));
#endif
}

static String toStringImpl(const NumericRange& range) {
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    for (const auto& dimension : range.dimensions()) {
        ss << dimension.min;
        if (dimension.min != dimension.max) {
            ss << ':' << dimension.max;
        }
        ss << ',';
    }
    auto str = ss.str();
    str.pop_back();  // remove last comma
    return String{str};
}

String NumericRange::toString() const {
    return toStringImpl(*this);
}

String toString(const NumericRange& range) {
    return toStringImpl(range);
}

#if UAPP_OPEN62541_VER_GE(1, 4)

/* ----------------------------------------- KeyValueMap ---------------------------------------- */
UA_KeyValueMap TypeHandler<UA_KeyValueMap>::copy(const UA_KeyValueMap& map) {
    UA_KeyValueMap result{};
    throwIfBad(UA_KeyValueMap_copy(&map, &result));
    return result;
}

// NOLINTNEXTLINE(*param-not-moved)
UA_KeyValueMap TypeHandler<UA_KeyValueMap>::move(UA_KeyValueMap&& map) noexcept {
    return std::exchange(map, {});
}

void TypeHandler<UA_KeyValueMap>::clear(UA_KeyValueMap& map) noexcept {
    UA_KeyValueMap_clear(&map);
}

KeyValueMap& KeyValueMap::set(const QualifiedName& key, const Variant& value) {
    throwIfBad(UA_KeyValueMap_set(handle(), key, value.handle()));
    return *this;
}

const Variant* KeyValueMap::get(const QualifiedName& key) const noexcept {
    const auto* x = UA_KeyValueMap_get(handle(), key);
    return asWrapper<Variant>(x);
}

bool KeyValueMap::contains(const QualifiedName& key) const noexcept {
    return UA_KeyValueMap_contains(handle(), *key.handle());
}

#endif

}  // namespace opcua
