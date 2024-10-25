#pragma once

#include "open62541pp/detail/open62541/client.h"

namespace opcua {
class Client;

namespace detail {
class ExceptionCatcher;
struct ClientContext;
}  // namespace detail
}  // namespace opcua

namespace opcua::detail {

UA_ClientConfig* getConfig(UA_Client* client) noexcept;
UA_Logger* getLogger(UA_ClientConfig* config) noexcept;
ClientContext* getContext(UA_Client* client) noexcept;
ClientContext& getContext(Client& client) noexcept;
ExceptionCatcher* getExceptionCatcher(UA_Client* client) noexcept;
ExceptionCatcher& getExceptionCatcher(Client& client) noexcept;
UA_Client* getHandle(Client& client) noexcept;

}  // namespace opcua::detail
