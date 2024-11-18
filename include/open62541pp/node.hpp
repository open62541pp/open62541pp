#pragma once

#include <cstdint>
#include <string_view>
#include <utility>  // move
#include <vector>

#include "open62541pp/bitmask.hpp"
#include "open62541pp/common.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/exception.hpp"
#include "open62541pp/nodeids.hpp"
#include "open62541pp/services/detail/async_transform.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/typeregistry.hpp"  // getDataType
#include "open62541pp/types.hpp"
#include "open62541pp/types_composed.hpp"
#include "open62541pp/wrapper.hpp"  // asWrapper

#include "open62541pp/services/attribute.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"
#include "open62541pp/services/method.hpp"
#include "open62541pp/services/nodemanagement.hpp"
#include "open62541pp/services/view.hpp"

namespace opcua {

/**
 * High-level node class to access node attribute, browse and populate address space.
 *
 * The Node API is just a more convenient way of using the free functions in the opcua::services
 * namespace.
 *
 * Node objects are useful as-is but they do not expose the entire OPC UA protocol. You can get
 * access to the associated NodeId instance with the Node::id() method and apply the native
 * open62541 functions or the free functions in the opcua::services namespace.
 *
 * @note The async functions are available for Client only.
 *
 * @tparam Connection Server or Client
 * @see Services
 */
template <typename Connection>
class Node {
public:
    /// Create a Node object.
    Node(Connection& connection, const NodeId& id)
        : connection_(&connection),
          id_(id) {}

    /// Create a Node object.
    Node(Connection& connection, NodeId&& id)
        : connection_(&connection),
          id_(std::move(id)) {}

    /// Get the server/client instance.
    Connection& connection() noexcept {
        return *connection_;
    }

    /// Get the server/client instance.
    const Connection& connection() const noexcept {
        return *connection_;
    }

    /// Get the node id.
    const NodeId& id() const noexcept {
        return id_;
    }

    /// Check if the Node exists in the most efficient manner.
    /// If the instance is of type `Node<Server>`, the internal node store is searched.
    /// If the instance is of type `Node<Client>`, an actual read request to the server is made.
    bool exists() noexcept;

    /// @name NodeManagement
    /// @{

    /// @wrapper{services::addFolder}
    Node addFolder(
        const NodeId& id,
        std::string_view browseName,
        const ObjectAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasComponent
    ) {
        return fromId(
            connection(),
            services::addFolder(connection(), this->id(), id, browseName, attributes, referenceType)
        );
    }

    /// @wrapper{services::addFolder}
    /// @param token @completiontoken{void(Result<Node>&)}
    /// @return @asyncresult{Result<Node>}
    template <typename CompletionToken = DefaultCompletionToken>
    auto addFolderAsync(
        const NodeId& id,
        std::string_view browseName,
        const ObjectAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasComponent,
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::addFolderAsync(
            connection(),
            this->id(),
            id,
            browseName,
            attributes,
            referenceType,
            fromIdAsync(connection(), std::forward<CompletionToken>(token))
        );
    }

    /// @wrapper{services::addObject}
    Node addObject(
        const NodeId& id,
        std::string_view browseName,
        const ObjectAttributes& attributes = {},
        const NodeId& objectType = ObjectTypeId::BaseObjectType,
        const NodeId& referenceType = ReferenceTypeId::HasComponent
    ) {
        return fromId(
            connection(),
            services::addObject(
                connection(), this->id(), id, browseName, attributes, objectType, referenceType
            )
        );
    }

    /// @wrapper{services::addObjectAsync}
    /// @param token @completiontoken{void(Result<Node>&)}
    /// @return @asyncresult{Result<Node>}
    template <typename CompletionToken = DefaultCompletionToken>
    auto addObjectAsync(
        const NodeId& id,
        std::string_view browseName,
        const ObjectAttributes& attributes = {},
        const NodeId& objectType = ObjectTypeId::BaseObjectType,
        const NodeId& referenceType = ReferenceTypeId::HasComponent,
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::addObjectAsync(
            connection(),
            this->id(),
            id,
            browseName,
            attributes,
            objectType,
            referenceType,
            fromIdAsync(connection(), std::forward<CompletionToken>(token))
        );
    }

    /// @wrapper{services::addVariable}
    Node addVariable(
        const NodeId& id,
        std::string_view browseName,
        const VariableAttributes& attributes = {},
        const NodeId& variableType = VariableTypeId::BaseDataVariableType,
        const NodeId& referenceType = ReferenceTypeId::HasComponent
    ) {
        return fromId(
            connection(),
            services::addVariable(
                connection(), this->id(), id, browseName, attributes, variableType, referenceType
            )
        );
    }

    /// @wrapper{services::addVariableAsync}
    /// @param token @completiontoken{void(Result<Node>&)}
    /// @return @asyncresult{Result<Node>}
    template <typename CompletionToken = DefaultCompletionToken>
    auto addVariableAsync(
        const NodeId& id,
        std::string_view browseName,
        const VariableAttributes& attributes = {},
        const NodeId& variableType = VariableTypeId::BaseDataVariableType,
        const NodeId& referenceType = ReferenceTypeId::HasComponent,
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::addVariableAsync(
            connection(),
            this->id(),
            id,
            browseName,
            attributes,
            variableType,
            referenceType,
            fromIdAsync(connection(), std::forward<CompletionToken>(token))
        );
    }

    /// @wrapper{services::addProperty}
    Node addProperty(
        const NodeId& id, std::string_view browseName, const VariableAttributes& attributes = {}
    ) {
        return fromId(
            connection(),
            services::addProperty(connection(), this->id(), id, browseName, attributes)
        );
    }

    /// @wrapper{services::addPropertyAsync}
    /// @param token @completiontoken{void(Result<Node>&)}
    /// @return @asyncresult{Result<Node>}
    template <typename CompletionToken = DefaultCompletionToken>
    auto addPropertyAsync(
        const NodeId& id,
        std::string_view browseName,
        const VariableAttributes& attributes = {},
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::addPropertyAsync(
            connection(),
            this->id(),
            id,
            browseName,
            attributes,
            fromIdAsync(connection(), std::forward<CompletionToken>(token))
        );
    }

#ifdef UA_ENABLE_METHODCALLS
    /// @wrapper{services::addMethod}
    Node addMethod(
        const NodeId& id,
        std::string_view browseName,
        services::MethodCallback callback,
        Span<const Argument> inputArguments,
        Span<const Argument> outputArguments,
        const MethodAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasComponent
    ) {
        return fromId(
            connection(),
            services::addMethod(
                connection(),
                this->id(),
                id,
                browseName,
                std::move(callback),
                inputArguments,
                outputArguments,
                attributes,
                referenceType
            )
        );
    }

    /// @wrapper{services::addMethodAsync}
    /// @param token @completiontoken{void(Result<Node>&)}
    /// @return @asyncresult{Result<Node>}
    template <typename CompletionToken = DefaultCompletionToken>
    auto addMethodAsync(
        const NodeId& id,
        std::string_view browseName,
        services::MethodCallback callback,
        Span<const Argument> inputArguments,
        Span<const Argument> outputArguments,
        const MethodAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasComponent,
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::addMethodAsync(
            connection(),
            this->id(),
            id,
            browseName,
            std::move(callback),
            inputArguments,
            outputArguments,
            attributes,
            referenceType,
            fromIdAsync(connection(), std::forward<CompletionToken>(token))
        );
    }
#endif

    /// @wrapper{services::addObjectType}
    Node addObjectType(
        const NodeId& id,
        std::string_view browseName,
        const ObjectTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        return fromId(
            connection(),
            services::addObjectType(
                connection(), this->id(), id, browseName, attributes, referenceType
            )
        );
    }

    /// @wrapper{services::addObjectTypeAsync}
    /// @param token @completiontoken{void(Result<Node>&)}
    /// @return @asyncresult{Result<Node>}
    template <typename CompletionToken = DefaultCompletionToken>
    auto addObjectTypeAsync(
        const NodeId& id,
        std::string_view browseName,
        const ObjectTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype,
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::addObjectTypeAsync(
            connection(),
            this->id(),
            id,
            browseName,
            attributes,
            referenceType,
            fromIdAsync(connection(), std::forward<CompletionToken>(token))
        );
    }

    /// @wrapper{services::addVariableType}
    Node addVariableType(
        const NodeId& id,
        std::string_view browseName,
        const VariableTypeAttributes& attributes = {},
        const NodeId& variableType = VariableTypeId::BaseDataVariableType,
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        return fromId(
            connection(),
            services::addVariableType(
                connection(), this->id(), id, browseName, attributes, variableType, referenceType
            )
        );
    }

    /// @wrapper{services::addVariableTypeAsync}
    /// @param token @completiontoken{void(Result<Node>&)}
    /// @return @asyncresult{Result<Node>}
    template <typename CompletionToken = DefaultCompletionToken>
    auto addVariableTypeAsync(
        const NodeId& id,
        std::string_view browseName,
        const VariableTypeAttributes& attributes = {},
        const NodeId& variableType = VariableTypeId::BaseDataVariableType,
        const NodeId& referenceType = ReferenceTypeId::HasSubtype,
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::addVariableTypeAsync(
            connection(),
            this->id(),
            id,
            browseName,
            attributes,
            variableType,
            referenceType,
            fromIdAsync(connection(), std::forward<CompletionToken>(token))
        );
    }

    /// @wrapper{services::addReferenceType}
    Node addReferenceType(
        const NodeId& id,
        std::string_view browseName,
        const ReferenceTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        return fromId(
            connection(),
            services::addReferenceType(
                connection(), this->id(), id, browseName, attributes, referenceType
            )
        );
    }

    /// @wrapper{services::addReferenceTypeAsync}
    /// @param token @completiontoken{void(Result<Node>&)}
    /// @return @asyncresult{Result<Node>}
    template <typename CompletionToken = DefaultCompletionToken>
    auto addReferenceTypeAsync(
        const NodeId& id,
        std::string_view browseName,
        const ReferenceTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype,
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::addReferenceTypeAsync(
            connection(),
            this->id(),
            id,
            browseName,
            attributes,
            referenceType,
            fromIdAsync(connection(), std::forward<CompletionToken>(token))
        );
    }

    /// @wrapper{services::addDataType}
    Node addDataType(
        const NodeId& id,
        std::string_view browseName,
        const DataTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        return fromId(
            connection(),
            services::addDataType(
                connection(), this->id(), id, browseName, attributes, referenceType
            )
        );
    }

    /// @wrapper{services::addDataTypeAsync}
    /// @param token @completiontoken{void(Result<Node>&)}
    /// @return @asyncresult{Result<Node>}
    template <typename CompletionToken = DefaultCompletionToken>
    auto addDataTypeAsync(
        const NodeId& id,
        std::string_view browseName,
        const DataTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype,
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::addDataTypeAsync(
            connection(),
            this->id(),
            id,
            browseName,
            attributes,
            referenceType,
            fromIdAsync(connection(), std::forward<CompletionToken>(token))
        );
    }

    /// @wrapper{services::addView}
    Node addView(
        const NodeId& id,
        std::string_view browseName,
        const ViewAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::Organizes
    ) {
        return fromId(
            connection(),
            services::addView(connection(), this->id(), id, browseName, attributes, referenceType)
        );
    }

    /// @wrapper{services::addViewAsync}
    /// @param token @completiontoken{void(Result<Node>&)}
    /// @return @asyncresult{Result<Node>}
    template <typename CompletionToken = DefaultCompletionToken>
    auto addViewAsync(
        const NodeId& id,
        std::string_view browseName,
        const ViewAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::Organizes,
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::addViewAsync(
            connection(),
            this->id(),
            id,
            browseName,
            attributes,
            referenceType,
            fromIdAsync(connection(), std::forward<CompletionToken>(token))
        );
    }

    /// @wrapper{services::addReference}
    Node& addReference(const NodeId& targetId, const NodeId& referenceType, bool forward = true) {
        services::addReference(connection(), id(), targetId, referenceType, forward).throwIfBad();
        return *this;
    }

    /// @wrapper{services::addReferenceAsync}
    /// @param token @completiontoken{void(StatusCode)}
    /// @return @asyncresult{StatusCode}
    template <typename CompletionToken = DefaultCompletionToken>
    auto addReferenceAsync(
        const NodeId& targetId,
        const NodeId& referenceType,
        bool forward = true,
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::addReferenceAsync(
            connection(),
            id(),
            targetId,
            referenceType,
            forward,
            std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::addModellingRule}
    Node& addModellingRule(ModellingRule rule) {
        services::addModellingRule(connection(), id(), rule).throwIfBad();
        return *this;
    }

    /// @wrapper{services::addModellingRuleAsync}
    /// @param token @completiontoken{void(StatusCode)}
    /// @return @asyncresult{StatusCode}
    template <typename CompletionToken = DefaultCompletionToken>
    auto addModellingRuleAsync(
        ModellingRule rule, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::addModellingRuleAsync(
            connection(), id(), rule, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::deleteNode}
    void deleteNode(bool deleteReferences = true) {
        services::deleteNode(connection(), id(), deleteReferences).throwIfBad();
    }

    /// @wrapper{services::deleteNodeAsync}
    /// @param token @completiontoken{void(StatusCode)}
    /// @return @asyncresult{StatusCode}
    template <typename CompletionToken = DefaultCompletionToken>
    auto deleteNodeAsync(
        bool deleteReferences = true, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::deleteNodeAsync(
            connection(), id(), deleteReferences, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::deleteReference}
    Node& deleteReference(
        const NodeId& targetId,
        const NodeId& referenceType,
        bool isForward = true,
        bool deleteBidirectional = true
    ) {
        services::deleteReference(
            connection(), id(), targetId, referenceType, isForward, deleteBidirectional
        )
            .throwIfBad();
        return *this;
    }

    /// @wrapper{services::deleteReferenceAsync}
    /// @param token @completiontoken{void(StatusCode)}
    /// @return @asyncresult{StatusCode}
    template <typename CompletionToken = DefaultCompletionToken>
    auto deleteReferenceAsync(
        const NodeId& targetId,
        const NodeId& referenceType,
        bool isForward = true,
        bool deleteBidirectional = true,
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::deleteReferenceAsync(
            connection(),
            id(),
            targetId,
            referenceType,
            isForward,
            deleteBidirectional,
            std::forward<CompletionToken>(token)
        );
    }

    /// @}
    /// @name View
    /// @{

    /// Browse references.
    std::vector<ReferenceDescription> browseReferences(
        BrowseDirection browseDirection = BrowseDirection::Both,
        const NodeId& referenceType = ReferenceTypeId::References,
        bool includeSubtypes = true,
        Bitmask<NodeClass> nodeClassMask = NodeClass::Unspecified
    ) {
        const BrowseDescription bd(
            id(),
            browseDirection,
            referenceType,
            includeSubtypes,
            nodeClassMask,
            BrowseResultMask::All
        );
        return services::browseAll(connection(), bd).value();
    }

    /// Browse referenced nodes (only local nodes).
    std::vector<Node> browseReferencedNodes(
        BrowseDirection browseDirection = BrowseDirection::Both,
        const NodeId& referenceType = ReferenceTypeId::References,
        bool includeSubtypes = true,
        Bitmask<NodeClass> nodeClassMask = NodeClass::Unspecified
    ) {
        const BrowseDescription bd(
            id(),
            browseDirection,
            referenceType,
            includeSubtypes,
            nodeClassMask,
            BrowseResultMask::TargetInfo  // only node id required here
        );
        auto refs = services::browseAll(connection(), bd).value();
        std::vector<Node> nodes;
        nodes.reserve(refs.size());
        for (auto&& ref : refs) {
            if (ref.getNodeId().isLocal()) {
                nodes.emplace_back(connection(), std::move(ref.getNodeId().getNodeId()));
            }
        }
        return nodes;
    }

    /// Browse child nodes (only local nodes).
    std::vector<Node> browseChildren(
        const NodeId& referenceType = ReferenceTypeId::HierarchicalReferences,
        Bitmask<NodeClass> nodeClassMask = NodeClass::Unspecified
    ) {
        return browseReferencedNodes(BrowseDirection::Forward, referenceType, true, nodeClassMask);
    }

    /// Browse child node specified by its relative path from this node (only local nodes).
    /// The relative path is specified using browse names.
    /// @exception BadStatus (BadNoMatch) If path not found
    Node browseChild(Span<const QualifiedName> path) {
        auto result = services::browseSimplifiedBrowsePath(connection(), id(), path);
        result.getStatusCode().throwIfBad();
        for (auto&& target : result.getTargets()) {
            if (target.getTargetId().isLocal()) {
                return {connection(), std::move(target.getTargetId().getNodeId())};
            }
        }
        throw BadStatus(UA_STATUSCODE_BADNOMATCH);
    }

    /// Browse parent node.
    /// A Node may have several parents, the first found is returned.
    /// @exception BadStatus (BadNotFound) If no parent node found
    Node browseParent() {
        const BrowseDescription bd(
            id(),
            BrowseDirection::Inverse,
            ReferenceTypeId::HierarchicalReferences,
            true,
            NodeClass::Unspecified,
            BrowseResultMask::TargetInfo
        );
        auto result = services::browse(connection(), bd, 1);
        result.getStatusCode().throwIfBad();
        if (result.getReferences().empty()) {
            throw BadStatus(UA_STATUSCODE_BADNOTFOUND);
        }
        return fromId(connection(), result.getReferences().front().getNodeId());
    }

#ifdef UA_ENABLE_METHODCALLS
    /// @}
    /// @name Method
    /// @{

    /// @wrapper{services::call}
    CallMethodResult callMethod(const NodeId& methodId, Span<const Variant> inputArguments) {
        return services::call(connection(), id(), methodId, inputArguments);
    }

    /// @wrapper{services::callAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto callMethodAsync(
        const NodeId& methodId,
        Span<const Variant> inputArguments,
        CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::callAsync(
            connection(), id(), methodId, inputArguments, std::forward<CompletionToken>(token)
        );
    }
#endif

    /// @}
    /// @name Attribute
    /// @{

    /// @wrapper{services::readNodeClass}
    NodeClass readNodeClass() {
        return services::readNodeClass(connection(), id()).value();
    }

    /// @wrapper{services::readNodeClassAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readNodeClassAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readNodeClassAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readBrowseName}
    QualifiedName readBrowseName() {
        return services::readBrowseName(connection(), id()).value();
    }

    /// @wrapper{services::readBrowseNameAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readBrowseNameAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readBrowseNameAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readDisplayName}
    LocalizedText readDisplayName() {
        return services::readDisplayName(connection(), id()).value();
    }

    /// @wrapper{services::readDisplayNameAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readDisplayNameAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readDisplayNameAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readDescription}
    LocalizedText readDescription() {
        return services::readDescription(connection(), id()).value();
    }

    /// @wrapper{services::readDescriptionAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readDescriptionAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readDescriptionAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readWriteMask}
    Bitmask<WriteMask> readWriteMask() {
        return services::readWriteMask(connection(), id()).value();
    }

    /// @wrapper{services::readWriteMaskAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readWriteMaskAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readWriteMaskAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readUserWriteMask}
    Bitmask<WriteMask> readUserWriteMask() {
        return services::readUserWriteMask(connection(), id()).value();
    }

    /// @wrapper{services::readUserWriteMaskAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readUserWriteMaskAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readUserWriteMaskAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readIsAbstract}
    bool readIsAbstract() {
        return services::readIsAbstract(connection(), id()).value();
    }

    /// @wrapper{services::readIsAbstractAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readIsAbstractAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readIsAbstractAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readSymmetric}
    bool readSymmetric() {
        return services::readSymmetric(connection(), id()).value();
    }

    /// @wrapper{services::readSymmetricAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readSymmetricAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readSymmetricAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readInverseName}
    LocalizedText readInverseName() {
        return services::readInverseName(connection(), id()).value();
    }

    /// @wrapper{services::readInverseNameAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readInverseNameAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readInverseNameAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readContainsNoLoops}
    bool readContainsNoLoops() {
        return services::readContainsNoLoops(connection(), id()).value();
    }

    /// @wrapper{services::readContainsNoLoopsAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readContainsNoLoopsAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readContainsNoLoopsAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readEventNotifier}
    Bitmask<EventNotifier> readEventNotifier() {
        return services::readEventNotifier(connection(), id()).value();
    }

    /// @wrapper{services::readEventNotifierAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readEventNotifierAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readEventNotifierAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readDataValue}
    DataValue readDataValue() {
        return services::readDataValue(connection(), id()).value();
    }

    /// @wrapper{services::readDataValueAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readDataValueAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readDataValueAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readValue}
    Variant readValue() {
        return services::readValue(connection(), id()).value();
    }

    /// @wrapper{services::readValueAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readValueAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readValueAsync(connection(), id(), std::forward<CompletionToken>(token));
    }

    /// Read scalar value from variable node.
    template <typename T>
    T readValueScalar() {
        return readValue().template getScalarCopy<T>();
    }

    /// Read array value from variable node.
    template <typename T>
    std::vector<T> readValueArray() {
        return readValue().template getArrayCopy<T>();
    }

    /// @wrapper{services::readDataType}
    NodeId readDataType() {
        return services::readDataType(connection(), id()).value();
    }

    /// @wrapper{services::readDataTypeAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readDataTypeAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readDataTypeAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readValueRank}
    ValueRank readValueRank() {
        return services::readValueRank(connection(), id()).value();
    }

    /// @wrapper{services::readValueRankAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readValueRankAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readValueRankAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readArrayDimensions}
    std::vector<uint32_t> readArrayDimensions() {
        return services::readArrayDimensions(connection(), id()).value();
    }

    /// @wrapper{services::readArrayDimensionsAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readArrayDimensionsAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readArrayDimensionsAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readAccessLevel}
    Bitmask<AccessLevel> readAccessLevel() {
        return services::readAccessLevel(connection(), id()).value();
    }

    /// @wrapper{services::readAccessLevelAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readAccessLevelAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readAccessLevelAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readUserAccessLevel}
    Bitmask<AccessLevel> readUserAccessLevel() {
        return services::readUserAccessLevel(connection(), id()).value();
    }

    /// @wrapper{services::readUserAccessLevelAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readUserAccessLevelAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readUserAccessLevelAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readMinimumSamplingInterval}
    double readMinimumSamplingInterval() {
        return services::readMinimumSamplingInterval(connection(), id()).value();
    }

    /// @wrapper{services::readMinimumSamplingIntervalAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readMinimumSamplingIntervalAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readMinimumSamplingIntervalAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readHistorizing}
    bool readHistorizing() {
        return services::readHistorizing(connection(), id()).value();
    }

    /// @wrapper{services::readHistorizingAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readHistorizingAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readHistorizingAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readExecutable}
    bool readExecutable() {
        return services::readExecutable(connection(), id()).value();
    }

    /// @wrapper{services::readExecutableAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readExecutableAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readExecutableAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readUserExecutable}
    bool readUserExecutable() {
        return services::readUserExecutable(connection(), id()).value();
    }

    /// @wrapper{services::readUserExecutableAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readUserExecutableAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readUserExecutableAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::readDataTypeDefinition}
    Variant readDataTypeDefinition() {
        return services::readDataTypeDefinition(connection(), id()).value();
    }

    /// @wrapper{services::readDataTypeDefinitionAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto readDataTypeDefinitionAsync(CompletionToken&& token = DefaultCompletionToken()) {
        return services::readDataTypeDefinitionAsync(
            connection(), id(), std::forward<CompletionToken>(token)
        );
    }

    /// Read the value of an object property.
    /// @param propertyName Browse name of the property (variable node)
    Variant readObjectProperty(const QualifiedName& propertyName) {
        return browseObjectProperty(propertyName).readValue();
    }

    /// @wrapper{services::writeDisplayName}
    Node& writeDisplayName(const LocalizedText& displayName) {
        services::writeDisplayName(connection(), id(), displayName).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeDisplayNameAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeDisplayNameAsync(
        const LocalizedText& displayName, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeDisplayNameAsync(
            connection(), id(), displayName, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeDescription}
    Node& writeDescription(const LocalizedText& description) {
        services::writeDescription(connection(), id(), description).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeDescriptionAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeDescriptionAsync(
        const LocalizedText& description, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeDescriptionAsync(
            connection(), id(), description, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeWriteMask}
    Node& writeWriteMask(Bitmask<WriteMask> writeMask) {
        services::writeWriteMask(connection(), id(), writeMask).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeWriteMaskAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeWriteMaskAsync(
        Bitmask<WriteMask> writeMask, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeWriteMaskAsync(
            connection(), id(), writeMask, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeUserWriteMask}
    Node& writeUserWriteMask(Bitmask<WriteMask> userWriteMask) {
        services::writeUserWriteMask(connection(), id(), userWriteMask).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeUserWriteMaskAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeUserWriteMaskAsync(
        Bitmask<WriteMask> userWriteMask, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeUserWriteMaskAsync(
            connection(), id(), userWriteMask, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeIsAbstract}
    Node& writeIsAbstract(bool isAbstract) {
        services::writeIsAbstract(connection(), id(), isAbstract).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeIsAbstractAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeIsAbstractAsync(bool isAbstract, CompletionToken&& token = DefaultCompletionToken()) {
        return services::writeIsAbstractAsync(
            connection(), id(), isAbstract, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeSymmetric}
    Node& writeSymmetric(bool symmetric) {
        services::writeSymmetric(connection(), id(), symmetric).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeSymmetricAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeSymmetricAsync(bool symmetric, CompletionToken&& token = DefaultCompletionToken()) {
        return services::writeSymmetricAsync(
            connection(), id(), symmetric, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeInverseName}
    Node& writeInverseName(const LocalizedText& inverseName) {
        services::writeInverseName(connection(), id(), inverseName).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeInverseNameAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeInverseNameAsync(
        const LocalizedText& inverseName, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeInverseNameAsync(
            connection(), id(), inverseName, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeContainsNoLoops}
    Node& writeContainsNoLoops(bool containsNoLoops) {
        services::writeContainsNoLoops(connection(), id(), containsNoLoops).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeContainsNoLoopsAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeContainsNoLoopsAsync(
        bool containsNoLoops, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeContainsNoLoopsAsync(
            connection(), id(), containsNoLoops, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeEventNotifier}
    Node& writeEventNotifier(Bitmask<EventNotifier> eventNotifier) {
        services::writeEventNotifier(connection(), id(), eventNotifier).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeEventNotifierAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeEventNotifierAsync(
        Bitmask<EventNotifier> eventNotifier, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeEventNotifierAsync(
            connection(), id(), eventNotifier, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeDataValue}
    Node& writeDataValue(const DataValue& value) {
        services::writeDataValue(connection(), id(), value).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeDataValueAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeDataValueAsync(
        const DataValue& value, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeDataValueAsync(
            connection(), id(), value, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeValue}
    Node& writeValue(const Variant& value) {
        services::writeValue(connection(), id(), value).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeValueAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeValueAsync(const Variant& value, CompletionToken&& token = DefaultCompletionToken()) {
        return services::writeValueAsync(
            connection(), id(), value, std::forward<CompletionToken>(token)
        );
    }

    /// Write scalar to variable node.
    template <typename T>
    Node& writeValueScalar(const T& value) {
        // NOLINTNEXTLINE(*-const-cast), variant isn't modified, try to avoid copy
        writeValue(Variant::fromScalar<VariantPolicy::ReferenceIfPossible>(const_cast<T&>(value)));
        return *this;
    }

    /// Write array value to variable node.
    template <typename ArrayLike>
    Node& writeValueArray(ArrayLike&& array) {
        writeValue(
            Variant::fromArray<VariantPolicy::ReferenceIfPossible>(std::forward<ArrayLike>(array))
        );
        return *this;
    }

    /// Write range of elements as array value to variable node.
    template <typename InputIt>
    Node& writeValueArray(InputIt first, InputIt last) {
        writeValue(Variant::fromArray<VariantPolicy::ReferenceIfPossible>(first, last));
        return *this;
    }

    /// @wrapper{services::writeDataType}
    Node& writeDataType(const NodeId& dataType) {
        services::writeDataType(connection(), id(), dataType).throwIfBad();
        return *this;
    }

    /// @overload
    /// Deduce the `typeId` from the template type.
    template <typename T>
    Node& writeDataType() {
        return writeDataType(asWrapper<NodeId>(getDataType<T>().typeId));
    }

    /// @wrapper{services::writeDataTypeAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeDataTypeAsync(
        const NodeId& dataType, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeDataTypeAsync(
            connection(), id(), dataType, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeValueRank}
    Node& writeValueRank(ValueRank valueRank) {
        services::writeValueRank(connection(), id(), valueRank).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeValueRankAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeValueRankAsync(
        ValueRank valueRank, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeValueRankAsync(
            connection(), id(), valueRank, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeArrayDimensions}
    Node& writeArrayDimensions(Span<const uint32_t> dimensions) {
        services::writeArrayDimensions(connection(), id(), dimensions).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeArrayDimensionsAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeArrayDimensionsAsync(
        Span<const uint32_t> dimensions, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeArrayDimensionsAsync(
            connection(), id(), dimensions, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeAccessLevel}
    Node& writeAccessLevel(Bitmask<AccessLevel> accessLevel) {
        services::writeAccessLevel(connection(), id(), accessLevel).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeAccessLevelAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeAccessLevelAsync(
        Bitmask<AccessLevel> accessLevel, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeAccessLevelAsync(
            connection(), id(), accessLevel, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeUserAccessLevel}
    Node& writeUserAccessLevel(Bitmask<AccessLevel> userAccessLevel) {
        services::writeUserAccessLevel(connection(), id(), userAccessLevel).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeUserAccessLevelAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeUserAccessLevelAsync(
        Bitmask<AccessLevel> userAccessLevel, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeUserAccessLevelAsync(
            connection(), id(), userAccessLevel, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeMinimumSamplingInterval}
    Node& writeMinimumSamplingInterval(double milliseconds) {
        services::writeMinimumSamplingInterval(connection(), id(), milliseconds).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeMinimumSamplingIntervalAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeMinimumSamplingIntervalAsync(
        double milliseconds, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeMinimumSamplingIntervalAsync(
            connection(), id(), milliseconds, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeHistorizing}
    Node& writeHistorizing(bool historizing) {
        services::writeHistorizing(connection(), id(), historizing).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeHistorizingAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeHistorizingAsync(
        bool historizing, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeHistorizingAsync(
            connection(), id(), historizing, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeExecutable}
    Node& writeExecutable(bool executable) {
        services::writeExecutable(connection(), id(), executable).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeExecutableAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeExecutableAsync(bool executable, CompletionToken&& token = DefaultCompletionToken()) {
        return services::writeExecutableAsync(
            connection(), id(), executable, std::forward<CompletionToken>(token)
        );
    }

    /// @wrapper{services::writeUserExecutable}
    Node& writeUserExecutable(bool userExecutable) {
        services::writeUserExecutable(connection(), id(), userExecutable).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeUserExecutableAsync}
    template <typename CompletionToken = DefaultCompletionToken>
    auto writeUserExecutableAsync(
        bool userExecutable, CompletionToken&& token = DefaultCompletionToken()
    ) {
        return services::writeUserExecutableAsync(
            connection(), id(), userExecutable, std::forward<CompletionToken>(token)
        );
    }

    /// Write the value of an object property.
    /// @param propertyName Browse name of the property (variable node)
    /// @param value New value
    Node& writeObjectProperty(const QualifiedName& propertyName, const Variant& value) {
        browseObjectProperty(propertyName).writeValue(value);
        return *this;
    }

    /// @}

private:
    static Node fromId(Connection& connection, Result<NodeId>&& result) {
        return {connection, std::move(result).value()};
    }

    static Node fromId(Connection& connection, ExpandedNodeId& id) {
        if (!id.isLocal()) {
            throw BadStatus(UA_STATUSCODE_BADNODEIDUNKNOWN);
        }
        return {connection, std::move(id.getNodeId())};
    }

    template <typename CompletionToken>
    static auto fromIdAsync(Connection& connection, CompletionToken&& token) {
        return services::detail::TransformToken(
            [&](Result<NodeId>& result) {
                return result.transform([&](NodeId& id) -> Node {
                    return {connection, std::move(id)};
                });
            },
            std::forward<CompletionToken>(token)
        );
    }

    Node browseObjectProperty(const QualifiedName& propertyName) {
        auto result = services::translateBrowsePathToNodeIds(
            connection(),
            BrowsePath(id(), {{ReferenceTypeId::HasProperty, false, true, propertyName}})
        );
        result.getStatusCode().throwIfBad();
        for (auto&& target : result.getTargets()) {
            if (target.getTargetId().isLocal()) {
                return {connection(), std::move(target.getTargetId().getNodeId())};
            }
        }
        throw BadStatus(UA_STATUSCODE_BADNOTFOUND);
    }

    Connection* connection_;
    NodeId id_;
};

/* ---------------------------------------------------------------------------------------------- */

template <typename Connection>
bool operator==(const Node<Connection>& lhs, const Node<Connection>& rhs) noexcept {
    return (lhs.connection() == rhs.connection()) && (lhs.id() == rhs.id());
}

template <typename Connection>
bool operator!=(const Node<Connection>& lhs, const Node<Connection>& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua
