#pragma once

#include <chrono>
#include <string_view>
#include <tuple>
#include <type_traits>

#include <doctest/doctest.h>

#include "open62541pp/client.hpp"
#include "open62541pp/server.hpp"

#include "async_trait.hpp"
#include "server_runner.hpp"

template <typename T>
constexpr bool isServer = std::is_same_v<std::remove_reference_t<T>, opcua::Server>;
template <typename T>
constexpr bool isClient = std::is_same_v<std::remove_reference_t<T>, opcua::Client>;

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

// Call the connection's runIterate function until predicate returns `true` or a timeout occurs.
template <typename Connection, typename UnaryPred>
bool runIterateUntil(
    Connection& connection, UnaryPred predicate, uint16_t timeoutMilliseconds = 1000
) {
    static_assert(std::is_same_v<std::invoke_result_t<UnaryPred>, bool>);

    const auto now = [] { return std::chrono::steady_clock::now(); };
    const auto startTime = now();

    while (!predicate()) {
        connection.runIterate();
        if ((now() - startTime) > std::chrono::milliseconds(timeoutMilliseconds)) {
            INFO("Timeout during runIterateUntil");
            return false;
        }
    }
    return true;
}
