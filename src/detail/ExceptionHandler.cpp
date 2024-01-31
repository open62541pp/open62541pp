#include "open62541pp/detail/ExceptionHandler.h"

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"

#include "../ClientContext.h"
#include "../ServerContext.h"

namespace opcua::detail {

ExceptionHandler& getExceptionHandler(ClientContext& context) noexcept {
    return context.exceptionHandler;
}

ExceptionHandler& getExceptionHandler(Client& client) noexcept {
    return getExceptionHandler(client.getContext());
}

ExceptionHandler& getExceptionHandler(ServerContext& context) noexcept {
    return context.exceptionHandler;
}

ExceptionHandler& getExceptionHandler(Server& server) noexcept {
    return getExceptionHandler(server.getContext());
}

}  // namespace opcua::detail
