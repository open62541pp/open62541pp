#include "open62541pp/services/nodemanagement.hpp"

#include <cassert>

#include "open62541pp/client.hpp"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/detail/result_utils.hpp"  // tryInvoke
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/detail/string_utils.hpp"  // detail::toNativeString
#include "open62541pp/exception.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/session.hpp"

namespace opcua::services {

AddNodesResponse addNodes(Client& connection, const AddNodesRequest& request) noexcept {
    return UA_Client_Service_addNodes(connection.handle(), request);
}

AddReferencesResponse addReferences(
    Client& connection, const AddReferencesRequest& request
) noexcept {
    return UA_Client_Service_addReferences(connection.handle(), request);
}

DeleteNodesResponse deleteNodes(Client& connection, const DeleteNodesRequest& request) noexcept {
    return UA_Client_Service_deleteNodes(connection.handle(), request);
}

DeleteReferencesResponse deleteReferences(
    Client& connection, const DeleteReferencesRequest& request
) noexcept {
    return UA_Client_Service_deleteReferences(connection.handle(), request);
}

template <>
Result<NodeId> addNode<Server>(
    Server& connection,
    NodeClass nodeClass,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceType
) noexcept {
    NodeId addedNodeId;
    const StatusCode status = __UA_Server_addNode(
        connection.handle(),
        static_cast<UA_NodeClass>(nodeClass),
        id.handle(),
        parentId.handle(),
        referenceType.handle(),
        {id.namespaceIndex(), opcua::detail::toNativeString(browseName)},
        typeDefinition.handle(),
        static_cast<const UA_NodeAttributes*>(nodeAttributes.decodedData()),
        nodeAttributes.decodedType(),
        nullptr,  // nodeContext
        addedNodeId.handle()
    );
    if (status.isBad()) {
        return BadResult{status};
    }
    return addedNodeId;
}

template <>
Result<NodeId> addNode<Client>(
    Client& connection,
    NodeClass nodeClass,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    const ExtensionObject& nodeAttributes,
    const NodeId& typeDefinition,
    const NodeId& referenceType
) noexcept {
    auto item = detail::makeAddNodesItem(
        parentId, referenceType, id, browseName, nodeClass, nodeAttributes, typeDefinition
    );
    const auto request = detail::makeAddNodesRequest(item);
    auto response = addNodes(connection, asWrapper<AddNodesRequest>(request));
    return detail::getSingleResultRef(response).andThen(detail::getAddedNodeId);
}

#ifdef UA_ENABLE_METHODCALLS

static UA_StatusCode methodCallback(
    UA_Server* server,
    const UA_NodeId* sessionId,
    void* sessionContext,
    const UA_NodeId* methodId,
    void* methodContext,
    const UA_NodeId* objectId,
    [[maybe_unused]] void* objectContext,
    size_t inputSize,
    const UA_Variant* input,
    size_t outputSize,
    UA_Variant* output
) noexcept {
    assert(methodContext != nullptr);
    const auto* nodeContext = static_cast<opcua::detail::NodeContext*>(methodContext);
    // simple callback signature with input, output only
    if (const auto* cb = std::get_if<0>(&nodeContext->methodCallback); cb != nullptr && *cb) {
        const auto result = opcua::detail::tryInvoke(
            *cb,
            Span<const Variant>{asWrapper<Variant>(input), inputSize},
            Span<Variant>{asWrapper<Variant>(output), outputSize}
        );
        return result.code();
    }
    // full callback signature
    if (const auto* cb = std::get_if<1>(&nodeContext->methodCallback); cb != nullptr && *cb) {
        assert(server != nullptr);
        assert(sessionId != nullptr);
        assert(methodId != nullptr);
        assert(objectId != nullptr);
        Session session{*asWrapper(server), asWrapper<NodeId>(*sessionId), sessionContext};
        const auto result = opcua::detail::tryInvoke(
            *cb,
            session,
            Span<const Variant>{asWrapper<Variant>(input), inputSize},
            Span<Variant>{asWrapper<Variant>(output), outputSize},
            asWrapper<NodeId>(*methodId),
            asWrapper<NodeId>(*objectId)
        );
        return result.code();
    }
    return UA_STATUSCODE_BADINTERNALERROR;
}

template <>
Result<NodeId> addMethod(
    Server& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    MethodCallback callback,
    Span<const Argument> inputArguments,
    Span<const Argument> outputArguments,
    const MethodAttributes& attributes,
    const NodeId& referenceType
) noexcept {
    return opcua::detail::tryInvoke([&] {
        auto* nodeContext = opcua::detail::getContext(connection).nodeContexts[id];
        nodeContext->methodCallback = std::move(callback);
        NodeId outputNodeId;
        throwIfBad(UA_Server_addMethodNode(
            connection.handle(),
            id,
            parentId,
            referenceType,
            {id.namespaceIndex(), opcua::detail::toNativeString(browseName)},
            attributes,
            methodCallback,
            inputArguments.size(),
            asNative(inputArguments.data()),
            outputArguments.size(),
            asNative(outputArguments.data()),
            nodeContext,
            outputNodeId.handle()  // outNewNodeId
        ));
        return outputNodeId;
    });
}

template <>
Result<NodeId> addMethod(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    [[maybe_unused]] MethodCallback callback,  // NOLINT(performance-unnecessary-value-param)
    [[maybe_unused]] Span<const Argument> inputArguments,
    [[maybe_unused]] Span<const Argument> outputArguments,
    const MethodAttributes& attributes,
    const NodeId& referenceType
) noexcept {
    return addNode(
        connection,
        NodeClass::Method,
        parentId,
        id,
        browseName,
        detail::wrapNodeAttributes(attributes),
        {},
        referenceType
    );
}
#endif

template <>
StatusCode addReference<Server>(
    Server& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward
) noexcept {
    return UA_Server_addReference(
        connection.handle(),
        sourceId,
        referenceType,
        {targetId, {}, 0},
        forward  // isForward
    );
}

template <>
StatusCode addReference<Client>(
    Client& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward
) noexcept {
    auto item = detail::makeAddReferencesItem(sourceId, referenceType, forward, targetId);
    const auto request = detail::makeAddReferencesRequest(item);
    return detail::getSingleStatus(
        addReferences(connection, asWrapper<AddReferencesRequest>(request))
    );
}

template <>
StatusCode deleteNode<Server>(
    Server& connection, const NodeId& id, bool deleteReferences
) noexcept {
    return UA_Server_deleteNode(connection.handle(), id, deleteReferences);
}

template <>
StatusCode deleteNode<Client>(
    Client& connection, const NodeId& id, bool deleteReferences
) noexcept {
    auto item = detail::makeDeleteNodesItem(id, deleteReferences);
    const auto request = detail::makeDeleteNodesRequest(item);
    return detail::getSingleStatus(deleteNodes(connection, asWrapper<DeleteNodesRequest>(request)));
}

template <>
StatusCode deleteReference<Server>(
    Server& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional
) noexcept {
    return UA_Server_deleteReference(
        connection.handle(),
        sourceId,
        referenceType,
        isForward,
        {targetId, {}, 0},
        deleteBidirectional
    );
}

template <>
StatusCode deleteReference<Client>(
    Client& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional
) noexcept {
    auto item = detail::makeDeleteReferencesItem(
        sourceId, referenceType, isForward, targetId, deleteBidirectional
    );
    const auto request = detail::makeDeleteReferencesRequest(item);
    return detail::getSingleStatus(
        deleteReferences(connection, asWrapper<DeleteReferencesRequest>(request))
    );
}

}  // namespace opcua::services
