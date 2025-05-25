#include "open62541pp/callback.hpp"

#include <cassert>
#include <utility>  // move

#include "open62541pp/client.hpp"
#include "open62541pp/detail/client_context.hpp"
#include "open62541pp/detail/client_utils.hpp"
#include "open62541pp/detail/result_utils.hpp"
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/detail/server_utils.hpp"
#include "open62541pp/server.hpp"

namespace opcua {

using CallbackContext = detail::Staleable<std::function<void()>>;

template <typename T>
struct CallbackFunctions;

template <>
struct CallbackFunctions<Client> {
    static constexpr auto addTimedCallback = &UA_Client_addTimedCallback;
    static constexpr auto addRepeatedCallback = &UA_Client_addRepeatedCallback;
    static constexpr auto changeRepeatedCallbackInterval = &UA_Client_changeRepeatedCallbackInterval;
    static constexpr auto removeCallback = &UA_Client_removeCallback;
};

template <>
struct CallbackFunctions<Server> {
    static constexpr auto addTimedCallback = &UA_Server_addTimedCallback;
    static constexpr auto addRepeatedCallback = &UA_Server_addRepeatedCallback;
    static constexpr auto changeRepeatedCallbackInterval = &UA_Server_changeRepeatedCallbackInterval;
    static constexpr auto removeCallback = &UA_Server_removeCallback;
};

template <typename T>
static void invokeCallback([[maybe_unused]] T* connection, void* data, bool remove) {
    assert(data != nullptr);
    auto* context = static_cast<CallbackContext*>(data);
    if (context->item) {
        [[maybe_unused]] const auto result = detail::tryInvoke(context->item);
    }
    if (remove) {
        context->stale = true;
        context->item = {};
    }
}

template <typename T>
static void timedCallback(T* connection, void* data) {
    return invokeCallback(connection, data, true);
}

template <typename T>
static void repeatedCallback(T* connection, void* data) {
    return invokeCallback(connection, data, false);
}

static UA_DateTime toMonotonic(UA_DateTime date) noexcept {
    const auto now = UA_DateTime_now();
    const auto nowMonotonic = UA_DateTime_nowMonotonic();
    const auto dateMonotonic = date - now + nowMonotonic;
    return std::max(dateMonotonic, UA_DateTime{0});
}

template <typename T>
CallbackId addTimedCallback(T& connection, TimedCallback callback, DateTime date) {
    auto context = std::make_unique<CallbackContext>(CallbackContext{false, std::move(callback)});
    CallbackId callbackId = 0;
    const auto status = CallbackFunctions<T>::addTimedCallback(
        connection.handle(), timedCallback, context.get(), toMonotonic(date), &callbackId
    );
    throwIfBad(status);
    detail::getContext(connection).callbacks.insert(callbackId, std::move(context));
    return callbackId;
}

template <typename T>
CallbackId addRepeatedCallback(T& connection, RepeatedCallback callback, double intervalMilliseconds) {
    auto context = std::make_unique<CallbackContext>(CallbackContext{false, std::move(callback)});
    CallbackId callbackId = 0;
    const auto status = CallbackFunctions<T>::addRepeatedCallback(
        connection.handle(), repeatedCallback, context.get(), intervalMilliseconds, &callbackId
    );
    throwIfBad(status);
    detail::getContext(connection).callbacks.insert(callbackId, std::move(context));
    return callbackId;
}

template <typename T>
void changeRepeatedCallbackInterval(
    T& connection, CallbackId callbackId, double intervalMilliseconds
) {
    const auto status = CallbackFunctions<T>::changeRepeatedCallbackInterval(
        connection.handle(), callbackId, intervalMilliseconds
    );
    throwIfBad(status);
}

template <typename T>
void removeCallback(T& connection, CallbackId callbackId) {
    CallbackFunctions<T>::removeCallback(connection.handle(), callbackId);
    detail::getContext(connection).callbacks.erase(callbackId);
}

// explicit template instantiations
template CallbackId addTimedCallback<Client>(Client&, TimedCallback, DateTime);
template CallbackId addTimedCallback<Server>(Server&, TimedCallback, DateTime);
template CallbackId addRepeatedCallback<Client>(Client&, RepeatedCallback, double);
template CallbackId addRepeatedCallback<Server>(Server&, RepeatedCallback, double);
template void changeRepeatedCallbackInterval<Client>(Client&, CallbackId, double);
template void changeRepeatedCallbackInterval<Server>(Server&, CallbackId, double);
template void removeCallback<Client>(Client&, CallbackId);
template void removeCallback<Server>(Server&, CallbackId);

}  // namespace opcua
