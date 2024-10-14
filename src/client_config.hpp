#pragma once

#include <memory>  // unique_ptr
#include <utility>  // forward, move
#include <vector>

#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/open62541/client.h"  // UA_ClientConfig
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/wrapper.hpp"

namespace opcua {

class ClientConfig : public Wrapper<UA_ClientConfig> {
public:
    ClientConfig() {
        throwIfBad(UA_ClientConfig_setDefault(handle()));
    }

    explicit ClientConfig(UA_ClientConfig&& native)
        : Wrapper(std::exchange(native, {})) {}

    ~ClientConfig() {
        // create temporary client to free config
        // reset callbacks to avoid notifications
        native().stateCallback = nullptr;
        native().inactivityCallback = nullptr;
        native().subscriptionInactivityCallback = nullptr;
#if UAPP_OPEN62541_VER_LE(1, 0)
        auto* client = UA_Client_new();
        auto* config = UA_Client_getConfig(client);
        detail::clear(config->logger);
        *config = std::exchange(native(), {});
#else
        auto* client = UA_Client_newWithConfig(handle());
#endif
        if (client != nullptr) {
            UA_Client_delete(client);
        }
    }

    void setUserIdentityToken(ExtensionObject token) {
        asWrapper<ExtensionObject>(native().userIdentityToken) = std::move(token);
    }

    template <typename T>
    void setUserIdentityToken(T&& token) {
        setUserIdentityToken(ExtensionObject::fromDecodedCopy(std::forward<T>(token)));
    }

    void setLogger(LogFunction func) {
        if (func) {
            auto adapter = std::make_unique<LoggerDefault>(std::move(func));
#if UAPP_OPEN62541_VER_GE(1, 4)
            assert(native().logging != nullptr);
            auto& logger = *native().logging;
#else
            auto& logger = native().logger;
#endif
            detail::clear(logger);
            logger = adapter->create(true);
            adapter.release();
        }
    }
};

}  // namespace opcua
