#pragma once

#include "open62541pp/Config.h"
#include "open62541pp/ValueBackend.h"
#include "open62541pp/services/NodeManagement.h"  // MethodCallback

namespace opcua::detail {

struct NodeContext {
    ValueCallback valueCallback;
    ValueBackendDataSource dataSource;
#ifdef UA_ENABLE_METHODCALLS
    services::MethodCallback methodCallback;
#endif
};

}  // namespace opcua::detail
