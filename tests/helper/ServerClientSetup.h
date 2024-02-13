#pragma once

#include <string_view>
#include <tuple>
#include <type_traits>

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"

#include "AsyncTrait.h"
#include "Runner.h"

template <typename T>
inline constexpr bool isServer = std::is_same_v<std::remove_reference_t<T>, opcua::Server>;
template <typename T>
inline constexpr bool isClient = std::is_same_v<std::remove_reference_t<T>, opcua::Client>;

struct ServerClientSetup {
    Client client;
    Server server;
    ServerRunner serverRunner{server};

    template <typename T>
    auto& getInstance() noexcept {
        if constexpr (IsTrait<T>::value) {
            using U = typename T::type;
            return std::get<U&>(std::tie(server, client));
        } else {
            return std::get<T&>(std::tie(server, client));
        }
    }

    static constexpr std::string_view endpointUrl = "opc.tcp://localhost:4840";
};
