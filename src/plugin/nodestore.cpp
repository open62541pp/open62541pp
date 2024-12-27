#include "open62541pp/plugin/nodestore.hpp"

#include <cassert>
#include <optional>

#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/detail/server_utils.hpp"
#include "open62541pp/exception.hpp"
#include "open62541pp/server.hpp"

namespace opcua {

static std::optional<Session> getSession(UA_Server* server, const UA_NodeId* sessionId) noexcept {
    auto* wrapper = asWrapper(server);
    if (wrapper == nullptr || sessionId == nullptr) {
        return std::nullopt;
    }
    return Session(*wrapper, asWrapper<NodeId>(*sessionId));
}

static void onReadNative(
    UA_Server* server,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    void* nodeContext,
    const UA_NumericRange* range,
    const UA_DataValue* value
) noexcept {
    assert(nodeContext != nullptr && nodeId != nullptr && value != nullptr);
    auto& callback = static_cast<detail::NodeContext*>(nodeContext)->valueCallback;
    auto* catcher = detail::getExceptionCatcher(server);
    if (callback != nullptr && catcher != nullptr) {
        catcher->invoke([&] {
            auto session = getSession(server, sessionId);
            callback->onRead(
                session.value(),
                asWrapper<NodeId>(*nodeId),
                asWrapper<NumericRange>(range),
                asWrapper<DataValue>(*value)
            );
        });
    }
}

static void onWriteNative(
    UA_Server* server,
    const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    const UA_NodeId* nodeId,
    void* nodeContext,
    const UA_NumericRange* range,
    const UA_DataValue* value
) noexcept {
    assert(nodeContext != nullptr && nodeId != nullptr && value != nullptr);
    auto& callback = static_cast<detail::NodeContext*>(nodeContext)->valueCallback;
    auto* catcher = detail::getExceptionCatcher(server);
    if (callback != nullptr && catcher != nullptr) {
        catcher->invoke([&] {
            auto session = getSession(server, sessionId);
            callback->onWrite(
                session.value(),
                asWrapper<NodeId>(*nodeId),
                asWrapper<NumericRange>(range),
                asWrapper<DataValue>(*value)
            );
        });
    }
}

UA_ValueCallback ValueCallbackBase::create(bool ownsAdapter) {
    if (ownsAdapter) {
        throw BadStatus(UA_STATUSCODE_BADINTERNALERROR);
    }
    UA_ValueCallback native{};
    native.onRead = onReadNative;
    native.onWrite = onWriteNative;
    return native;
}

}  // namespace opcua
