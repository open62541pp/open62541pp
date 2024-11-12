#pragma once

#include <cstdint>
#include <string_view>
#include <utility>  // move
#include <vector>

#include "open62541pp/bitmask.hpp"
#include "open62541pp/common.hpp"  // BrowseDirection
#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/exception.hpp"
#include "open62541pp/nodeids.hpp"
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

    /// @wrapper{services::addProperty}
    Node addProperty(
        const NodeId& id, std::string_view browseName, const VariableAttributes& attributes = {}
    ) {
        return fromId(
            connection(),
            services::addProperty(connection(), this->id(), id, browseName, attributes)
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

    /// @wrapper{services::addReference}
    Node& addReference(const NodeId& targetId, const NodeId& referenceType, bool forward = true) {
        services::addReference(connection(), id(), targetId, referenceType, forward).throwIfBad();
        return *this;
    }

    /// @wrapper{services::addModellingRule}
    Node& addModellingRule(ModellingRule rule) {
        services::addModellingRule(connection(), id(), rule).throwIfBad();
        return *this;
    }

    /// @wrapper{services::deleteNode}
    void deleteNode(bool deleteReferences = true) {
        services::deleteNode(connection(), id(), deleteReferences).throwIfBad();
    }

    /// @wrapper{services::deleteReference}
    Node& deleteReference(
        const NodeId& targetId,
        const NodeId& referenceType,
        bool isForward,
        bool deleteBidirectional
    ) {
        services::deleteReference(
            connection(), id(), targetId, referenceType, isForward, deleteBidirectional
        )
            .throwIfBad();
        return *this;
    }

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
    /// Call a server method.
    /// @param methodId NodeId of the method (`HasComponent` reference to current node required)
    /// @param inputArguments Input argument values
    CallMethodResult callMethod(const NodeId& methodId, Span<const Variant> inputArguments) {
        return services::call(connection(), id(), methodId, inputArguments);
    }
#endif

    /// @wrapper{services::readNodeClass}
    NodeClass readNodeClass() {
        return services::readNodeClass(connection(), id()).value();
    }

    /// @wrapper{services::readBrowseName}
    QualifiedName readBrowseName() {
        return services::readBrowseName(connection(), id()).value();
    }

    /// @wrapper{services::readDisplayName}
    LocalizedText readDisplayName() {
        return services::readDisplayName(connection(), id()).value();
    }

    /// @wrapper{services::readDescription}
    LocalizedText readDescription() {
        return services::readDescription(connection(), id()).value();
    }

    /// @wrapper{services::readWriteMask}
    Bitmask<WriteMask> readWriteMask() {
        return services::readWriteMask(connection(), id()).value();
    }

    /// @wrapper{services::readUserWriteMask}
    Bitmask<WriteMask> readUserWriteMask() {
        return services::readUserWriteMask(connection(), id()).value();
    }

    /// @wrapper{services::readIsAbstract}
    bool readIsAbstract() {
        return services::readIsAbstract(connection(), id()).value();
    }

    /// @wrapper{services::readSymmetric}
    bool readSymmetric() {
        return services::readSymmetric(connection(), id()).value();
    }

    /// @wrapper{services::readInverseName}
    LocalizedText readInverseName() {
        return services::readInverseName(connection(), id()).value();
    }

    /// @wrapper{services::readContainsNoLoops}
    bool readContainsNoLoops() {
        return services::readContainsNoLoops(connection(), id()).value();
    }

    /// @wrapper{services::readEventNotifier}
    Bitmask<EventNotifier> readEventNotifier() {
        return services::readEventNotifier(connection(), id()).value();
    }

    /// @wrapper{services::readDataValue}
    DataValue readDataValue() {
        return services::readDataValue(connection(), id()).value();
    }

    /// @wrapper{services::readValue}
    Variant readValue() {
        return services::readValue(connection(), id()).value();
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

    /// @wrapper{services::readValueRank}
    ValueRank readValueRank() {
        return services::readValueRank(connection(), id()).value();
    }

    /// @wrapper{services::readArrayDimensions}
    std::vector<uint32_t> readArrayDimensions() {
        return services::readArrayDimensions(connection(), id()).value();
    }

    /// @wrapper{services::readAccessLevel}
    Bitmask<AccessLevel> readAccessLevel() {
        return services::readAccessLevel(connection(), id()).value();
    }

    /// @wrapper{services::readUserAccessLevel}
    Bitmask<AccessLevel> readUserAccessLevel() {
        return services::readUserAccessLevel(connection(), id()).value();
    }

    /// @wrapper{services::readMinimumSamplingInterval}
    double readMinimumSamplingInterval() {
        return services::readMinimumSamplingInterval(connection(), id()).value();
    }

    /// @wrapper{services::readHistorizing}
    bool readHistorizing() {
        return services::readHistorizing(connection(), id()).value();
    }

    /// @wrapper{services::readExecutable}
    bool readExecutable() {
        return services::readExecutable(connection(), id()).value();
    }

    /// @wrapper{services::readUserExecutable}
    bool readUserExecutable() {
        return services::readUserExecutable(connection(), id()).value();
    }

    /// @wrapper{services::readDataTypeDefinition}
    Variant readDataTypeDefinition() {
        return services::readDataTypeDefinition(connection(), id()).value();
    }

    /// Read the value of an object property.
    /// @param propertyName Browse name of the property (variable node)
    Variant readObjectProperty(const QualifiedName& propertyName) {
        return browseObjectProperty(propertyName).readValue();
    }

    /// @wrapper{services::writeDisplayName}
    Node& writeDisplayName(const LocalizedText& name) {
        services::writeDisplayName(connection(), id(), name).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeDescription}
    Node& writeDescription(const LocalizedText& desc) {
        services::writeDescription(connection(), id(), desc).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeWriteMask}
    Node& writeWriteMask(Bitmask<WriteMask> mask) {
        services::writeWriteMask(connection(), id(), mask).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeWriteMask}
    Node& writeUserWriteMask(Bitmask<WriteMask> mask) {
        services::writeUserWriteMask(connection(), id(), mask).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeIsAbstract}
    Node& writeIsAbstract(bool isAbstract) {
        services::writeIsAbstract(connection(), id(), isAbstract).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeSymmetric}
    Node& writeSymmetric(bool symmetric) {
        services::writeSymmetric(connection(), id(), symmetric).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeInverseName}
    Node& writeInverseName(const LocalizedText& name) {
        services::writeInverseName(connection(), id(), name).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeContainsNoLoops}
    Node& writeContainsNoLoops(bool containsNoLoops) {
        services::writeContainsNoLoops(connection(), id(), containsNoLoops).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeEventNotifier}
    Node& writeEventNotifier(Bitmask<EventNotifier> mask) {
        services::writeEventNotifier(connection(), id(), mask).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeDataValue}
    Node& writeDataValue(const DataValue& value) {
        services::writeDataValue(connection(), id(), value).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeValue}
    Node& writeValue(const Variant& value) {
        services::writeValue(connection(), id(), value).throwIfBad();
        return *this;
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
    Node& writeDataType(const NodeId& typeId) {
        services::writeDataType(connection(), id(), typeId).throwIfBad();
        return *this;
    }

    /// @overload
    /// Deduce the `typeId` from the template type.
    template <typename T>
    Node& writeDataType() {
        return writeDataType(asWrapper<NodeId>(getDataType<T>().typeId));
    }

    /// @wrapper{services::writeValueRank}
    Node& writeValueRank(ValueRank valueRank) {
        services::writeValueRank(connection(), id(), valueRank).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeArrayDimensions}
    Node& writeArrayDimensions(Span<const uint32_t> dimensions) {
        services::writeArrayDimensions(connection(), id(), dimensions).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeAccessLevel}
    Node& writeAccessLevel(Bitmask<AccessLevel> mask) {
        services::writeAccessLevel(connection(), id(), mask).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeUserAccessLevel}
    Node& writeUserAccessLevel(Bitmask<AccessLevel> mask) {
        services::writeUserAccessLevel(connection(), id(), mask).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeMinimumSamplingInterval}
    Node& writeMinimumSamplingInterval(double milliseconds) {
        services::writeMinimumSamplingInterval(connection(), id(), milliseconds).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeHistorizing}
    Node& writeHistorizing(bool historizing) {
        services::writeHistorizing(connection(), id(), historizing).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeExecutable}
    Node& writeExecutable(bool executable) {
        services::writeExecutable(connection(), id(), executable).throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeUserExecutable}
    Node& writeUserExecutable(bool userExecutable) {
        services::writeUserExecutable(connection(), id(), userExecutable).throwIfBad();
        return *this;
    }

    /// Write the value of an object property.
    /// @param propertyName Browse name of the property (variable node)
    /// @param value New value
    Node& writeObjectProperty(const QualifiedName& propertyName, const Variant& value) {
        browseObjectProperty(propertyName).writeValue(value);
        return *this;
    }

private:
    static Node fromId(Connection& connection, Result<NodeId>&& result) {
        return {connection, std::move(result).value()};
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
