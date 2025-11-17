#include "open62541pp/plugin/nodesetloader.hpp"

#if UAPP_HAS_NODESETLOADER

#include <string>

#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/server.hpp"
#include "open62541pp/types.hpp"

namespace opcua {

StatusCode loadNodeset(Server& server, std::string_view nodeset2XmlFilePath) {
    return UA_Server_loadNodeset(
        server.handle(), std::string{nodeset2XmlFilePath}.c_str(), nullptr
    );
}

}  // namespace opcua

#endif
