#pragma once

#include <optional>

#include "open62541pp/detail/open62541/server.h"

namespace opcua {
class Server;
class Session;

namespace detail {
class ExceptionCatcher;
struct ServerContext;
}  // namespace detail
}  // namespace opcua

namespace opcua::detail {

UA_ServerConfig* getConfig(UA_Server* server) noexcept;
UA_Logger* getLogger(UA_ServerConfig* config) noexcept;
ServerContext* getContext(UA_Server* server) noexcept;
ServerContext& getContext(Server& server) noexcept;
ExceptionCatcher* getExceptionCatcher(UA_Server* server) noexcept;
ExceptionCatcher& getExceptionCatcher(Server& server) noexcept;
UA_Server* getHandle(Server& server) noexcept;
std::optional<Session> getSession(
    UA_Server* server, const UA_NodeId* sessionId, void* sessionContext
) noexcept;

}  // namespace opcua::detail
