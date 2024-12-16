#pragma once

#include <chrono>
#include <string_view>
#include <tuple>
#include <type_traits>

#include "open62541pp/client.hpp"
#include "open62541pp/server.hpp"

#include "async_trait.hpp"
#include "server_runner.hpp"

template <typename T>
constexpr bool isServer = std::is_same_v<std::remove_reference_t<T>, opcua::Server>;
template <typename T>
constexpr bool isClient = std::is_same_v<std::remove_reference_t<T>, opcua::Client>;

enum class ConditionStatus { occurred, timeout };

// Call the connection's runIterate function until either condition or a timeout occurred.
template <typename Connection, typename UnaryPred>
ConditionStatus runIterateUntil(
    Connection& connection, UnaryPred predicate, long timeoutMilliseconds = 1000
) {
    static_assert(std::is_same_v<std::invoke_result_t<UnaryPred>, bool>);
    auto nowInMilliseconds = []() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        )
            .count();
    };

    auto currentTime = nowInMilliseconds();
    long duration{};
    bool conditionTrue = false;
    do {
        connection.runIterate();
        conditionTrue = predicate();
        auto nextTime = nowInMilliseconds();
        duration = nextTime - currentTime;
        currentTime = nextTime;
    } while (!conditionTrue && (duration < timeoutMilliseconds));

    return conditionTrue ? ConditionStatus::occurred : ConditionStatus::timeout;
}

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
