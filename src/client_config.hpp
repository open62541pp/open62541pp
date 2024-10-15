#pragma once

#include <memory>  // unique_ptr
#include <utility>  // forward, move
#include <vector>

#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/open62541/client.h"  // UA_ClientConfig
#include "open62541pp/exception.hpp"
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/wrapper.hpp"

namespace opcua {

namespace detail {
inline void clear(UA_ClientConfig& config) noexcept {
    // create temporary client to free config
    // reset callbacks to avoid notifications
    config.stateCallback = nullptr;
    config.inactivityCallback = nullptr;
    config.subscriptionInactivityCallback = nullptr;
#if UAPP_OPEN62541_VER_LE(1, 0)
    auto* client = UA_Client_new();
    auto* configClient = UA_Client_getConfig(client);
    clear(configClient->logger);
    *configClient = config;
#else
    auto* client = UA_Client_newWithConfig(&config);
#endif
    if (client != nullptr) {
        UA_Client_delete(client);
    }
}
}  // namespace detail

class ClientConfig : public Wrapper<UA_ClientConfig> {
public:
    ClientConfig() {
        throwIfBad(UA_ClientConfig_setDefault(handle()));
    }

#ifdef UA_ENABLE_ENCRYPTION
    ClientConfig(
        const ByteString& certificate,
        const ByteString& privateKey,
        Span<const ByteString> trustList,
        Span<const ByteString> revocationList = {}
    ) {
        throwIfBad(UA_ClientConfig_setDefaultEncryption(
            handle(),
            certificate,
            privateKey,
            asNative(trustList.data()),
            trustList.size(),
            asNative(revocationList.data()),
            revocationList.size()
        ));
    }
#endif

    explicit ClientConfig(UA_ClientConfig&& native)
        : Wrapper(std::exchange(native, {})) {}

    ~ClientConfig() {
        detail::clear(native());
    }

    ClientConfig(const ClientConfig&) = delete;

    ClientConfig(ClientConfig&& config) noexcept
        : Wrapper(std::exchange(config.native(), {})) {}

    ClientConfig& operator=(const ClientConfig&) = delete;

    ClientConfig& operator=(ClientConfig&& other) noexcept {
        if (this != &other) {
            native() = std::exchange(other.native(), {});
        }
        return *this;
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
