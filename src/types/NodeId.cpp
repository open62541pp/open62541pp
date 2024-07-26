#include "open62541pp/types/NodeId.h"

namespace opcua {

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
        result.append("s=").append(getIdentifierAs<String>().get());
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
