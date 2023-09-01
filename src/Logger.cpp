#include "open62541pp/Logger.h"

#include <cstdarg>  // va_list
#include <string>

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"

#include "open62541_impl.h"

namespace opcua {

inline static const UA_Logger& getLogger(UA_Client& client) {
    return UA_Client_getConfig(&client)->logger;
}

inline static const UA_Logger& getLogger(UA_Server& server) {
    return UA_Server_getConfig(&server)->logger;
}

inline static const UA_Logger& getLogger(Client& client) {
    return getLogger(*client.handle());
}

inline static const UA_Logger& getLogger(Server& server) {
    return getLogger(*server.handle());
}

template <typename T>
inline static void logImpl(
    T& serverOrClient, LogLevel level, LogCategory category, std::string_view msg
) {
    const auto& logger = getLogger(serverOrClient);
    if (logger.log == nullptr) {
        return;
    }
    va_list args{};  // NOLINT
    logger.log(
        logger.context,
        static_cast<UA_LogLevel>(level),
        static_cast<UA_LogCategory>(category),
        std::string(msg).c_str(),
        args  // NOLINT
    );
}

void log(UA_Client* client, LogLevel level, LogCategory category, std::string_view msg) {
    if (client != nullptr) {
        logImpl(*client, level, category, msg);
    }
}

void log(Client& client, LogLevel level, LogCategory category, std::string_view msg) {
    logImpl(client, level, category, msg);
}

void log(UA_Server* server, LogLevel level, LogCategory category, std::string_view msg) {
    if (server != nullptr) {
        logImpl(*server, level, category, msg);
    }
}

void log(Server& server, LogLevel level, LogCategory category, std::string_view msg) {
    logImpl(server, level, category, msg);
}

}  // namespace opcua
