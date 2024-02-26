#pragma once

#include <cstdint>
#include <string_view>
#include <utility>  // move
#include <vector>

#include "open62541pp/Bitmask.h"
#include "open62541pp/Common.h"
#include "open62541pp/Config.h"
#include "open62541pp/NodeIds.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeRegistry.h"  // getDataType
#include "open62541pp/Wrapper.h"  // asWrapper
#include "open62541pp/open62541.h"
#include "open62541pp/services/Attribute.h"
#include "open62541pp/services/Method.h"
#include "open62541pp/services/NodeManagement.h"
#include "open62541pp/services/View.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/Composed.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

namespace opcua {

/**
 * High-level node class to access node attribute, browse and populate address space.
 *
 * The Node API is just a more convenient way of using the free functions in the `services`
 * namespace.
 *
 * Node objects are useful as-is but they do not expose the entire OPC UA protocol. You can get
 * access to the associated NodeId instance with the getNodeId() method and apply the native
 * open62541 functions or the free functions in the `services` namespace.
 *
 * @see Services
 */
template <typename ServerOrClient>
class Node {
public:
    /// Create a Node object.
    Node(ServerOrClient& connection, const NodeId& id)
        : connection_(connection),
          nodeId_(id) {}

    /// Create a Node object.
    Node(ServerOrClient& connection, NodeId&& id)
        : connection_(connection),
          nodeId_(std::move(id)) {}

    /// Get the server/client instance.
    ServerOrClient& getConnection() noexcept {
        return connection_;
    }

    /// Get the server/client instance.
    const ServerOrClient& getConnection() const noexcept {
        return connection_;
    }

    /// Get the node id.
    const NodeId& getNodeId() const noexcept {
        return nodeId_;
    }

    /// Check if the Node exists in the most efficient manner.
    /// If the instance is of type `Node<Server>`, the internal node store is searched.
    /// If the instance is of type `Node<Client>`, an actual read request to the server is made.
    bool exists() noexcept;

    /// @copydoc services::addFolder
    Node addFolder(
        const NodeId& id,
        std::string_view browseName,
        const ObjectAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasComponent
    ) {
        NodeId resultingId = services::addFolder(
            connection_, nodeId_, id, browseName, attributes, referenceType
        );
        return {connection_, resultingId};
    }

    /// @copydoc services::addObject
    Node addObject(
        const NodeId& id,
        std::string_view browseName,
        const ObjectAttributes& attributes = {},
        const NodeId& objectType = ObjectTypeId::BaseObjectType,
        const NodeId& referenceType = ReferenceTypeId::HasComponent
    ) {
        NodeId resultingId = services::addObject(
            connection_, nodeId_, id, browseName, attributes, objectType, referenceType
        );
        return {connection_, resultingId};
    }

    /// @copydoc services::addVariable
    Node addVariable(
        const NodeId& id,
        std::string_view browseName,
        const VariableAttributes& attributes = {},
        const NodeId& variableType = VariableTypeId::BaseDataVariableType,
        const NodeId& referenceType = ReferenceTypeId::HasComponent
    ) {
        NodeId resultingId = services::addVariable(
            connection_, nodeId_, id, browseName, attributes, variableType, referenceType
        );
        return {connection_, resultingId};
    }

    /// @copydoc services::addProperty
    Node addProperty(
        const NodeId& id, std::string_view browseName, const VariableAttributes& attributes = {}
    ) {
        NodeId resultingId = services::addProperty(
            connection_, nodeId_, id, browseName, attributes
        );
        return {connection_, resultingId};
    }

#ifdef UA_ENABLE_METHODCALLS
    /// @copydoc services::addMethod
    Node addMethod(
        const NodeId& id,
        std::string_view browseName,
        services::MethodCallback callback,
        Span<const Argument> inputArguments,
        Span<const Argument> outputArguments,
        const MethodAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasComponent
    ) {
        NodeId resultingId = services::addMethod(
            connection_,
            nodeId_,
            id,
            browseName,
            std::move(callback),
            inputArguments,
            outputArguments,
            attributes,
            referenceType
        );
        return {connection_, resultingId};
    }
#endif

    /// @copydoc services::addObjectType
    Node addObjectType(
        const NodeId& id,
        std::string_view browseName,
        const ObjectTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        NodeId resultingId = services::addObjectType(
            connection_, nodeId_, id, browseName, attributes, referenceType
        );
        return {connection_, resultingId};
    }

    /// @copydoc services::addVariableType
    Node addVariableType(
        const NodeId& id,
        std::string_view browseName,
        const VariableTypeAttributes& attributes = {},
        const NodeId& variableType = VariableTypeId::BaseDataVariableType,
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        NodeId resultingId = services::addVariableType(
            connection_, nodeId_, id, browseName, attributes, variableType, referenceType
        );
        return {connection_, resultingId};
    }

    /// @copydoc services::addReferenceType
    Node addReferenceType(
        const NodeId& id,
        std::string_view browseName,
        const ReferenceTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        NodeId resultingId = services::addReferenceType(
            connection_, nodeId_, id, browseName, attributes, referenceType
        );
        return {connection_, resultingId};
    }

    /// @copydoc services::addDataType
    Node addDataType(
        const NodeId& id,
        std::string_view browseName,
        const DataTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        NodeId resultingId = services::addDataType(
            connection_, nodeId_, id, browseName, attributes, referenceType
        );
        return {connection_, resultingId};
    }

    /// @copydoc services::addView
    Node addView(
        const NodeId& id,
        std::string_view browseName,
        const ViewAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::Organizes
    ) {
        NodeId resultingId = services::addView(
            connection_, nodeId_, id, browseName, attributes, referenceType
        );
        return {connection_, resultingId};
    }

    /// @copydoc services::addReference
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& addReference(const NodeId& targetId, const NodeId& referenceType, bool forward = true) {
        services::addReference(connection_, nodeId_, targetId, referenceType, forward);
        return *this;
    }

    /// @copydoc services::addModellingRule
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& addModellingRule(ModellingRule rule) {
        services::addModellingRule(connection_, nodeId_, rule);
        return *this;
    }

    /// @copydoc services::deleteNode
    void deleteNode(bool deleteReferences = true) {
        services::deleteNode(connection_, nodeId_, deleteReferences);
    }

    /// @copydoc services::deleteReference
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& deleteReference(
        const NodeId& targetId,
        const NodeId& referenceType,
        bool isForward,
        bool deleteBidirectional
    ) {
        services::deleteReference(
            connection_, nodeId_, targetId, referenceType, isForward, deleteBidirectional
        );
        return *this;
    }

    /// Browse references.
    std::vector<ReferenceDescription> browseReferences(
        BrowseDirection browseDirection = BrowseDirection::Both,
        const NodeId& referenceType = ReferenceTypeId::References,
        bool includeSubtypes = true,
        Bitmask<NodeClass> nodeClassMask = NodeClass::Unspecified
    ) {
        return services::browseAll(
            connection_,
            BrowseDescription(
                nodeId_,
                browseDirection,
                referenceType,
                includeSubtypes,
                nodeClassMask,
                BrowseResultMask::All
            )
        );
    }

    /// Browse referenced nodes (only local nodes).
    std::vector<Node> browseReferencedNodes(
        BrowseDirection browseDirection = BrowseDirection::Both,
        const NodeId& referenceType = ReferenceTypeId::References,
        bool includeSubtypes = true,
        Bitmask<NodeClass> nodeClassMask = NodeClass::Unspecified
    ) {
        auto refs = services::browseAll(
            connection_,
            BrowseDescription(
                nodeId_,
                browseDirection,
                referenceType,
                includeSubtypes,
                nodeClassMask,
                BrowseResultMask::TargetInfo  // only node id required here
            )
        );
        std::vector<Node> nodes;
        nodes.reserve(refs.size());
        for (auto&& ref : refs) {
            if (ref.getNodeId().isLocal()) {
                nodes.emplace_back(connection_, std::move(ref.getNodeId().getNodeId()));
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
        auto result = services::browseSimplifiedBrowsePath(connection_, nodeId_, path);
        for (auto&& target : result.getTargets()) {
            if (target.getTargetId().isLocal()) {
                return {connection_, std::move(target.getTargetId().getNodeId())};
            }
        }
        throw BadStatus(UA_STATUSCODE_BADNOMATCH);
    }

    /// Browse parent node.
    /// A Node may have several parents, the first found is returned.
    /// @exception BadStatus (BadNotFound) If no parent node found
    Node browseParent() {
        const auto nodes = browseReferencedNodes(
            BrowseDirection::Inverse,
            ReferenceTypeId::HierarchicalReferences,
            true,
            UA_NODECLASS_UNSPECIFIED
        );
        if (nodes.empty()) {
            throw BadStatus(UA_STATUSCODE_BADNOTFOUND);
        }
        return nodes[0];
    }

#ifdef UA_ENABLE_METHODCALLS
    /// Call a server method and return results.
    /// @param methodId NodeId of the method (`HasComponent` reference to current node required)
    /// @param inputArguments Input argument values
    std::vector<Variant> callMethod(const NodeId& methodId, Span<const Variant> inputArguments) {
        return services::call(connection_, nodeId_, methodId, inputArguments);
    }
#endif

    /// @copydoc services::readNodeClass
    NodeClass readNodeClass() {
        return services::readNodeClass(connection_, nodeId_);
    }

    /// @copydoc services::readBrowseName
    QualifiedName readBrowseName() {
        return services::readBrowseName(connection_, nodeId_);
    }

    /// @copydoc services::readDisplayName
    LocalizedText readDisplayName() {
        return services::readDisplayName(connection_, nodeId_);
    }

    /// @copydoc services::readDescription
    LocalizedText readDescription() {
        return services::readDescription(connection_, nodeId_);
    }

    /// @copydoc services::readWriteMask
    Bitmask<WriteMask> readWriteMask() {
        return services::readWriteMask(connection_, nodeId_);
    }

    /// @copydoc services::readUserWriteMask
    Bitmask<WriteMask> readUserWriteMask() {
        return services::readUserWriteMask(connection_, nodeId_);
    }

    /// @copydoc services::readIsAbstract
    bool readIsAbstract() {
        return services::readIsAbstract(connection_, nodeId_);
    }

    /// @copydoc services::readSymmetric
    bool readSymmetric() {
        return services::readSymmetric(connection_, nodeId_);
    }

    /// @copydoc services::readInverseName
    LocalizedText readInverseName() {
        return services::readInverseName(connection_, nodeId_);
    }

    /// @copydoc services::readContainsNoLoops
    bool readContainsNoLoops() {
        return services::readContainsNoLoops(connection_, nodeId_);
    }

    /// @copydoc services::readEventNotifier
    Bitmask<EventNotifier> readEventNotifier() {
        return services::readEventNotifier(connection_, nodeId_);
    }

    /// @copydoc services::readDataValue
    DataValue readDataValue() {
        return services::readDataValue(connection_, nodeId_);
    }

    /// @copydoc services::readValue
    Variant readValue() {
        return services::readValue(connection_, nodeId_);
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

    /// @copydoc services::readDataType
    NodeId readDataType() {
        return services::readDataType(connection_, nodeId_);
    }

    /// @copydoc services::readValueRank
    ValueRank readValueRank() {
        return services::readValueRank(connection_, nodeId_);
    }

    /// @copydoc services::readArrayDimensions
    std::vector<uint32_t> readArrayDimensions() {
        return services::readArrayDimensions(connection_, nodeId_);
    }

    /// @copydoc services::readAccessLevel
    Bitmask<AccessLevel> readAccessLevel() {
        return services::readAccessLevel(connection_, nodeId_);
    }

    /// @copydoc services::readUserAccessLevel
    Bitmask<AccessLevel> readUserAccessLevel() {
        return services::readUserAccessLevel(connection_, nodeId_);
    }

    /// @copydoc services::readMinimumSamplingInterval
    double readMinimumSamplingInterval() {
        return services::readMinimumSamplingInterval(connection_, nodeId_);
    }

    /// @copydoc services::readHistorizing
    bool readHistorizing() {
        return services::readHistorizing(connection_, nodeId_);
    }

    /// @copydoc services::readExecutable
    bool readExecutable() {
        return services::readExecutable(connection_, nodeId_);
    }

    /// @copydoc services::readUserExecutable
    bool readUserExecutable() {
        return services::readUserExecutable(connection_, nodeId_);
    }

    /// Read the value of an object property.
    /// @param propertyName Browse name of the property (variable node)
    Variant readObjectProperty(const QualifiedName& propertyName) {
        return browseObjectProperty(propertyName).readValue();
    }

    /// @copydoc services::writeDisplayName
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeDisplayName(const LocalizedText& name) {
        services::writeDisplayName(connection_, nodeId_, name);
        return *this;
    }

    /// @copydoc services::writeDescription
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeDescription(const LocalizedText& desc) {
        services::writeDescription(connection_, nodeId_, desc);
        return *this;
    }

    /// @copydoc services::writeWriteMask
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeWriteMask(Bitmask<WriteMask> mask) {
        services::writeWriteMask(connection_, nodeId_, mask);
        return *this;
    }

    /// @copydoc services::writeWriteMask
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeUserWriteMask(Bitmask<WriteMask> mask) {
        services::writeUserWriteMask(connection_, nodeId_, mask);
        return *this;
    }

    /// @copydoc services::writeIsAbstract
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeIsAbstract(bool isAbstract) {
        services::writeIsAbstract(connection_, nodeId_, isAbstract);
        return *this;
    }

    /// @copydoc services::writeSymmetric
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeSymmetric(bool symmetric) {
        services::writeSymmetric(connection_, nodeId_, symmetric);
        return *this;
    }

    /// @copydoc services::writeInverseName
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeInverseName(const LocalizedText& name) {
        services::writeInverseName(connection_, nodeId_, name);
        return *this;
    }

    /// @copydoc services::writeContainsNoLoops
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeContainsNoLoops(bool containsNoLoops) {
        services::writeContainsNoLoops(connection_, nodeId_, containsNoLoops);
        return *this;
    }

    /// @copydoc services::writeEventNotifier
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeEventNotifier(Bitmask<EventNotifier> mask) {
        services::writeEventNotifier(connection_, nodeId_, mask);
        return *this;
    }

    /// @copydoc services::writeDataValue
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeDataValue(const DataValue& value) {
        services::writeDataValue(connection_, nodeId_, value);
        return *this;
    }

    /// @copydoc services::writeValue
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeValue(const Variant& value) {
        services::writeValue(connection_, nodeId_, value);
        return *this;
    }

    /// Write scalar to variable node.
    /// @return Current node instance to chain multiple methods (fluent interface)
    template <typename T>
    Node& writeValueScalar(const T& value) {
        // NOLINTNEXTLINE, variant isn't modified, try to avoid copy
        writeValue(Variant::fromScalar<VariantPolicy::ReferenceIfPossible>(const_cast<T&>(value)));
        return *this;
    }

    /// Write array value to variable node.
    /// @return Current node instance to chain multiple methods (fluent interface)
    template <typename ArrayLike>
    Node& writeValueArray(ArrayLike&& array) {
        writeValue(
            Variant::fromArray<VariantPolicy::ReferenceIfPossible>(std::forward<ArrayLike>(array))
        );
        return *this;
    }

    /// Write range of elements as array value to variable node.
    /// @return Current node instance to chain multiple methods (fluent interface)
    template <typename InputIt>
    Node& writeValueArray(InputIt first, InputIt last) {
        writeValue(Variant::fromArray<VariantPolicy::ReferenceIfPossible>(first, last));
        return *this;
    }

    /// @copydoc services::writeDataType
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeDataType(const NodeId& typeId) {
        services::writeDataType(connection_, nodeId_, typeId);
        return *this;
    }

    /// @overload
    /// Deduce the `typeId` from the template type.
    /// @return Current node instance to chain multiple methods (fluent interface)
    template <typename T>
    Node& writeDataType() {
        return writeDataType(asWrapper<NodeId>(getDataType<T>().typeId));
    }

    /// @copydoc services::writeValueRank
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeValueRank(ValueRank valueRank) {
        services::writeValueRank(connection_, nodeId_, valueRank);
        return *this;
    }

    /// @copydoc services::writeArrayDimensions
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeArrayDimensions(Span<const uint32_t> dimensions) {
        services::writeArrayDimensions(connection_, nodeId_, dimensions);
        return *this;
    }

    /// @copydoc services::writeAccessLevel
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeAccessLevel(Bitmask<AccessLevel> mask) {
        services::writeAccessLevel(connection_, nodeId_, mask);
        return *this;
    }

    /// @copydoc services::writeUserAccessLevel
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeUserAccessLevel(Bitmask<AccessLevel> mask) {
        services::writeUserAccessLevel(connection_, nodeId_, mask);
        return *this;
    }

    /// @copydoc services::writeMinimumSamplingInterval
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeMinimumSamplingInterval(double milliseconds) {
        services::writeMinimumSamplingInterval(connection_, nodeId_, milliseconds);
        return *this;
    }

    /// @copydoc services::writeHistorizing
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeHistorizing(bool historizing) {
        services::writeHistorizing(connection_, nodeId_, historizing);
        return *this;
    }

    /// @copydoc services::writeExecutable
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeExecutable(bool executable) {
        services::writeExecutable(connection_, nodeId_, executable);
        return *this;
    }

    /// @copydoc services::writeUserExecutable
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeUserExecutable(bool userExecutable) {
        services::writeUserExecutable(connection_, nodeId_, userExecutable);
        return *this;
    }

    /// Write the value of an object property.
    /// @param propertyName Browse name of the property (variable node)
    /// @param value New value
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeObjectProperty(const QualifiedName& propertyName, const Variant& value) {
        browseObjectProperty(propertyName).writeValue(value);
        return *this;
    }

private:
    Node browseObjectProperty(const QualifiedName& propertyName) {
        auto result = services::translateBrowsePathToNodeIds(
            connection_,
            BrowsePath(nodeId_, {{ReferenceTypeId::HasProperty, false, true, propertyName}})
        );
        result.getStatusCode().throwIfBad();
        for (auto&& target : result.getTargets()) {
            if (target.getTargetId().isLocal()) {
                return {connection_, std::move(target.getTargetId().getNodeId())};
            }
        }
        throw BadStatus(UA_STATUSCODE_BADNOTFOUND);
    }

    ServerOrClient& connection_;
    NodeId nodeId_;
};

/* ---------------------------------------------------------------------------------------------- */

template <typename ServerOrClient>
bool operator==(const Node<ServerOrClient>& lhs, const Node<ServerOrClient>& rhs) noexcept {
    return (lhs.getConnection() == rhs.getConnection()) && (lhs.getNodeId() == rhs.getNodeId());
}

template <typename ServerOrClient>
bool operator!=(const Node<ServerOrClient>& lhs, const Node<ServerOrClient>& rhs) noexcept {
    return !(lhs == rhs);
}

}  // namespace opcua
