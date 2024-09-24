#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/NodeId.h"

#include <ctime>  // gmtime, localtime
#include <iomanip>  // put_time
#include <ostream>
#include <sstream>

#include "open62541pp/Config.h"

namespace opcua {

/* ------------------------------------------- String ------------------------------------------- */

std::ostream& operator<<(std::ostream& os, const String& str) {
    os << static_cast<std::string_view>(str);
    return os;
}

/* -------------------------------------------- Guid -------------------------------------------- */

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

/* ------------------------------------------ DateTime ------------------------------------------ */

std::string DateTime::format(std::string_view format, bool localtime) const {
    const std::time_t unixTime = toUnixTime();
    std::ostringstream ss;
    const auto* timeinfo = localtime ? std::localtime(&unixTime) : std::gmtime(&unixTime);
    if (timeinfo != nullptr) {
        ss << std::put_time(timeinfo, std::string(format).c_str());
    }
    return ss.str();
}

/* ----------------------------------------- ByteString ----------------------------------------- */

ByteString ByteString::fromBase64([[maybe_unused]] std::string_view encoded) {
#if UAPP_OPEN62541_VER_GE(1, 1)
    ByteString output;
    UA_ByteString_fromBase64(output.handle(), String(encoded).handle());
    return output;
#else
    return {};
#endif
}

// NOLINTNEXTLINE
std::string ByteString::toBase64() const {
#if UAPP_OPEN62541_VER_GE(1, 1)
    String output;
    UA_ByteString_toBase64(handle(), output.handle());
    return std::string(output);
#else
    return {};
#endif
}

/* ----------------------------------------- XmlElement ----------------------------------------- */

std::ostream& operator<<(std::ostream& os, const XmlElement& xmlElement) {
    os << static_cast<std::string_view>(xmlElement);
    return os;
}

/* ---------------------------------------- NumericRange ---------------------------------------- */

NumericRange::NumericRange(std::string_view encodedRange) {
    const UA_String encodedRangeNative = detail::toNativeString(encodedRange);
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

/* ------------------------------------------- NodeId ------------------------------------------- */

std::string NodeId::toString() const {
    std::string result;
    const auto ns = getNamespaceIndex();
    if (ns > 0) {
        result.append("ns=").append(std::to_string(ns)).append(";");
    }
    switch (getIdentifierType()) {
    case NodeIdType::Numeric:
        result.append("i=").append(std::to_string(getIdentifierAs<uint32_t>()));
        break;
    case NodeIdType::String:
        result.append("s=").append(getIdentifierAs<String>());
        break;
    case NodeIdType::Guid:
        result.append("g=").append(getIdentifierAs<Guid>().toString());
        break;
    case NodeIdType::ByteString:
        result.append("b=").append(getIdentifierAs<ByteString>().toBase64());
        break;
    }
    return result;
}

/* --------------------------------------- ExpandedNodeId --------------------------------------- */

std::string ExpandedNodeId::toString() const {
    std::string result;
    const auto svr = getServerIndex();
    if (svr > 0) {
        result.append("svr=").append(std::to_string(svr)).append(";");
    }
    const auto nsu = getNamespaceUri();
    if (!nsu.empty()) {
        result.append("nsu=").append(nsu).append(";");
    }
    result.append(getNodeId().toString());
    return result;
}

}  // namespace opcua
