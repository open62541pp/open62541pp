#include "open62541pp/services/nodemanagement.hpp"

#include <cassert>

#include "open62541pp/client.hpp"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/detail/result_utils.hpp"  // tryInvoke
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/detail/string_utils.hpp"  // detail::toNativeString
#include "open62541pp/exception.hpp"
#include "open62541pp/server.hpp"

namespace opcua::services {

AddNodesResponse addNodes(Client& connection, const AddNodesRequest& request) noexcept {
    return addNodesAsync(connection, request, detail::SyncOperation{});
}

AddReferencesResponse addReferences(
    Client& connection, const AddReferencesRequest& request
) noexcept {
    return addReferencesAsync(connection, request, detail::SyncOperation{});
}

DeleteNodesResponse deleteNodes(Client& connection, const DeleteNodesRequest& request) noexcept {
    return deleteNodesAsync(connection, request, detail::SyncOperation{});
}

DeleteReferencesResponse deleteReferences(
    Client& connection, const DeleteReferencesRequest& request
) noexcept {
    return deleteReferencesAsync(connection, request, detail::SyncOperation{});
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
        {id.getNamespaceIndex(), opcua::detail::toNativeString(browseName)},
        typeDefinition.handle(),
        static_cast<const UA_NodeAttributes*>(nodeAttributes.getDecodedData()),
        nodeAttributes.getDecodedDataType(),
        nullptr,  // nodeContext
        addedNodeId.handle()
    );
    if (status.isBad()) {
        return BadResult(status);
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
    return addNodeAsync(
        connection,
        nodeClass,
        parentId,
        id,
        browseName,
        nodeAttributes,
        typeDefinition,
        referenceType,
        detail::SyncOperation{}
    );
}

#ifdef UA_ENABLE_METHODCALLS

static UA_StatusCode methodCallback(
    [[maybe_unused]] UA_Server* server,
    [[maybe_unused]] const UA_NodeId* sessionId,
    [[maybe_unused]] void* sessionContext,
    [[maybe_unused]] const UA_NodeId* methodId,
    void* methodContext,
    [[maybe_unused]] const UA_NodeId* objectId,
    [[maybe_unused]] void* objectContext,
    size_t inputSize,
    const UA_Variant* input,
    size_t outputSize,
    UA_Variant* output
) noexcept {
    assert(methodContext != nullptr);
    const auto* nodeContext = static_cast<opcua::detail::NodeContext*>(methodContext);
    const auto& callback = nodeContext->methodCallback;
    if (callback) {
        return opcua::detail::tryInvoke(
                   callback,
                   Span<const Variant>{asWrapper<Variant>(input), inputSize},
                   Span<Variant>{asWrapper<Variant>(output), outputSize}
        )
            .code();
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
        const auto status = UA_Server_addMethodNode(
            connection.handle(),
            id,
            parentId,
            referenceType,
            {id.getNamespaceIndex(), opcua::detail::toNativeString(browseName)},
            attributes,
            methodCallback,
            inputArguments.size(),
            asNative(inputArguments.data()),
            outputArguments.size(),
            asNative(outputArguments.data()),
            nodeContext,
            outputNodeId.handle()  // outNewNodeId
        );
        throwIfBad(status);
        return outputNodeId;
    });
}

template <>
Result<NodeId> addMethod(
    Client& connection,
    const NodeId& parentId,
    const NodeId& id,
    std::string_view browseName,
    MethodCallback callback,
    Span<const Argument> inputArguments,
    Span<const Argument> outputArguments,
    const MethodAttributes& attributes,
    const NodeId& referenceType
) noexcept {
    return addMethodAsync(
        connection,
        parentId,
        id,
        browseName,
        std::move(callback),
        inputArguments,
        outputArguments,
        attributes,
        referenceType,
        detail::SyncOperation{}
    );
}
#endif

template <>
Result<void> addReference<Server>(
    Server& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward
) noexcept {
    return detail::toResult(UA_Server_addReference(
        connection.handle(),
        sourceId,
        referenceType,
        {targetId, {}, 0},
        forward  // isForward
    ));
}

template <>
Result<void> addReference<Client>(
    Client& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool forward
) noexcept {
    return addReferenceAsync(
        connection, sourceId, targetId, referenceType, forward, detail::SyncOperation{}
    );
}

template <>
Result<void> deleteNode<Server>(
    Server& connection, const NodeId& id, bool deleteReferences
) noexcept {
    return detail::toResult(UA_Server_deleteNode(connection.handle(), id, deleteReferences));
}

template <>
Result<void> deleteNode<Client>(
    Client& connection, const NodeId& id, bool deleteReferences
) noexcept {
    return deleteNodeAsync(connection, id, deleteReferences, detail::SyncOperation{});
}

template <>
Result<void> deleteReference<Server>(
    Server& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional
) noexcept {
    return detail::toResult(UA_Server_deleteReference(
        connection.handle(),
        sourceId,
        referenceType,
        isForward,
        {targetId, {}, 0},
        deleteBidirectional
    ));
}

template <>
Result<void> deleteReference<Client>(
    Client& connection,
    const NodeId& sourceId,
    const NodeId& targetId,
    const NodeId& referenceType,
    bool isForward,
    bool deleteBidirectional
) noexcept {
    return deleteReferenceAsync(
        connection,
        sourceId,
        targetId,
        referenceType,
        isForward,
        deleteBidirectional,
        detail::SyncOperation{}
    );
}

}  // namespace opcua::services
