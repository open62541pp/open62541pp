#include "open62541pp/detail/ExceptionCatcher.h"

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"
#include "open62541pp/detail/ClientContext.h"
#include "open62541pp/detail/ServerContext.h"

namespace opcua::detail {

ExceptionCatcher& getExceptionCatcher(ClientContext& context) noexcept {
    return context.exceptionCatcher;
}

ExceptionCatcher& getExceptionCatcher(Client& client) noexcept {
    return getExceptionCatcher(client.getContext());
}

ExceptionCatcher& getExceptionCatcher(ServerContext& context) noexcept {
    return context.exceptionCatcher;
}

ExceptionCatcher& getExceptionCatcher(Server& server) noexcept {
    return getExceptionCatcher(server.getContext());
}

}  // namespace opcua::detail
