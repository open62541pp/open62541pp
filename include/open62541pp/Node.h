#pragma once

#include <cstdint>
#include <string_view>
#include <utility>  // move
#include <vector>

#include "open62541pp/Bitmask.h"
#include "open62541pp/Common.h"  // BrowseDirection
#include "open62541pp/Config.h"
#include "open62541pp/NodeIds.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeRegistry.h"  // getDataType
#include "open62541pp/Wrapper.h"  // asWrapper
#include "open62541pp/detail/open62541/common.h"
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
        : connection_(connection),
          id_(id) {}

    /// Create a Node object.
    Node(Connection& connection, NodeId&& id)
        : connection_(connection),
          id_(std::move(id)) {}

    /// Get the server/client instance.
    Connection& connection() noexcept {
        return connection_;
    }

    /// Get the server/client instance.
    const Connection& connection() const noexcept {
        return connection_;
    }

    /// @deprecated Use connection() instead
    [[deprecated("Use connection() instead")]]
    Connection& getConnection() noexcept {
        return connection_;
    }

    /// @deprecated Use connection() instead
    [[deprecated("Use connection() instead")]]
    const Connection& getConnection() const noexcept {
        return connection_;
    }

    /// Get the node id.
    const NodeId& id() const noexcept {
        return id_;
    }

    /// @deprecated Use id() instead
    [[deprecated("Use id() instead")]]
    const NodeId& getNodeId() const noexcept {
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
        auto result = services::addFolder(
            connection_, id_, id, browseName, attributes, referenceType
        );
        return {connection_, result.value()};
    }

    /// @wrapper{services::addObject}
    Node addObject(
        const NodeId& id,
        std::string_view browseName,
        const ObjectAttributes& attributes = {},
        const NodeId& objectType = ObjectTypeId::BaseObjectType,
        const NodeId& referenceType = ReferenceTypeId::HasComponent
    ) {
        auto result = services::addObject(
            connection_, id_, id, browseName, attributes, objectType, referenceType
        );
        return {connection_, result.value()};
    }

    /// @wrapper{services::addVariable}
    Node addVariable(
        const NodeId& id,
        std::string_view browseName,
        const VariableAttributes& attributes = {},
        const NodeId& variableType = VariableTypeId::BaseDataVariableType,
        const NodeId& referenceType = ReferenceTypeId::HasComponent
    ) {
        auto result = services::addVariable(
            connection_, id_, id, browseName, attributes, variableType, referenceType
        );
        return {connection_, result.value()};
    }

    /// @wrapper{services::addProperty}
    Node addProperty(
        const NodeId& id, std::string_view browseName, const VariableAttributes& attributes = {}
    ) {
        auto result = services::addProperty(connection_, id_, id, browseName, attributes);
        return {connection_, result.value()};
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
        auto result = services::addMethod(
            connection_,
            id_,
            id,
            browseName,
            std::move(callback),
            inputArguments,
            outputArguments,
            attributes,
            referenceType
        );
        return {connection_, result.value()};
    }
#endif

    /// @wrapper{services::addObjectType}
    Node addObjectType(
        const NodeId& id,
        std::string_view browseName,
        const ObjectTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        auto result = services::addObjectType(
            connection_, id_, id, browseName, attributes, referenceType
        );
        return {connection_, result.value()};
    }

    /// @wrapper{services::addVariableType}
    Node addVariableType(
        const NodeId& id,
        std::string_view browseName,
        const VariableTypeAttributes& attributes = {},
        const NodeId& variableType = VariableTypeId::BaseDataVariableType,
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        auto result = services::addVariableType(
            connection_, id_, id, browseName, attributes, variableType, referenceType
        );
        return {connection_, result.value()};
    }

    /// @wrapper{services::addReferenceType}
    Node addReferenceType(
        const NodeId& id,
        std::string_view browseName,
        const ReferenceTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        auto result = services::addReferenceType(
            connection_, id_, id, browseName, attributes, referenceType
        );
        return {connection_, result.value()};
    }

    /// @wrapper{services::addDataType}
    Node addDataType(
        const NodeId& id,
        std::string_view browseName,
        const DataTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        auto result = services::addDataType(
            connection_, id_, id, browseName, attributes, referenceType
        );
        return {connection_, result.value()};
    }

    /// @wrapper{services::addView}
    Node addView(
        const NodeId& id,
        std::string_view browseName,
        const ViewAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::Organizes
    ) {
        auto result = services::addView(
            connection_, id_, id, browseName, attributes, referenceType
        );
        return {connection_, result.value()};
    }

    /// @wrapper{services::addReference}
    Node& addReference(const NodeId& targetId, const NodeId& referenceType, bool forward = true) {
        services::addReference(connection_, id_, targetId, referenceType, forward)
            .code()
            .throwIfBad();
        return *this;
    }

    /// @wrapper{services::addModellingRule}
    Node& addModellingRule(ModellingRule rule) {
        services::addModellingRule(connection_, id_, rule).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::deleteNode}
    void deleteNode(bool deleteReferences = true) {
        services::deleteNode(connection_, id_, deleteReferences).code().throwIfBad();
    }

    /// @wrapper{services::deleteReference}
    Node& deleteReference(
        const NodeId& targetId,
        const NodeId& referenceType,
        bool isForward,
        bool deleteBidirectional
    ) {
        services::deleteReference(
            connection_, id_, targetId, referenceType, isForward, deleteBidirectional
        )
            .code()
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
            id_,
            browseDirection,
            referenceType,
            includeSubtypes,
            nodeClassMask,
            BrowseResultMask::All
        );
        return services::browseAll(connection_, bd).value();
    }

    /// Browse referenced nodes (only local nodes).
    std::vector<Node> browseReferencedNodes(
        BrowseDirection browseDirection = BrowseDirection::Both,
        const NodeId& referenceType = ReferenceTypeId::References,
        bool includeSubtypes = true,
        Bitmask<NodeClass> nodeClassMask = NodeClass::Unspecified
    ) {
        const BrowseDescription bd(
            id_,
            browseDirection,
            referenceType,
            includeSubtypes,
            nodeClassMask,
            BrowseResultMask::TargetInfo  // only node id required here
        );
        auto refs = services::browseAll(connection_, bd).value();
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
        auto result = services::browseSimplifiedBrowsePath(connection_, id_, path).value();
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
        return services::call(connection_, id_, methodId, inputArguments).value();
    }
#endif

    /// @wrapper{services::readNodeClass}
    NodeClass readNodeClass() {
        return services::readNodeClass(connection_, id_).value();
    }

    /// @wrapper{services::readBrowseName}
    QualifiedName readBrowseName() {
        return services::readBrowseName(connection_, id_).value();
    }

    /// @wrapper{services::readDisplayName}
    LocalizedText readDisplayName() {
        return services::readDisplayName(connection_, id_).value();
    }

    /// @wrapper{services::readDescription}
    LocalizedText readDescription() {
        return services::readDescription(connection_, id_).value();
    }

    /// @wrapper{services::readWriteMask}
    Bitmask<WriteMask> readWriteMask() {
        return services::readWriteMask(connection_, id_).value();
    }

    /// @wrapper{services::readUserWriteMask}
    Bitmask<WriteMask> readUserWriteMask() {
        return services::readUserWriteMask(connection_, id_).value();
    }

    /// @wrapper{services::readIsAbstract}
    bool readIsAbstract() {
        return services::readIsAbstract(connection_, id_).value();
    }

    /// @wrapper{services::readSymmetric}
    bool readSymmetric() {
        return services::readSymmetric(connection_, id_).value();
    }

    /// @wrapper{services::readInverseName}
    LocalizedText readInverseName() {
        return services::readInverseName(connection_, id_).value();
    }

    /// @wrapper{services::readContainsNoLoops}
    bool readContainsNoLoops() {
        return services::readContainsNoLoops(connection_, id_).value();
    }

    /// @wrapper{services::readEventNotifier}
    Bitmask<EventNotifier> readEventNotifier() {
        return services::readEventNotifier(connection_, id_).value();
    }

    /// @wrapper{services::readDataValue}
    DataValue readDataValue() {
        return services::readDataValue(connection_, id_).value();
    }

    /// @wrapper{services::readValue}
    Variant readValue() {
        return services::readValue(connection_, id_).value();
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
        return services::readDataType(connection_, id_).value();
    }

    /// @wrapper{services::readValueRank}
    ValueRank readValueRank() {
        return services::readValueRank(connection_, id_).value();
    }

    /// @wrapper{services::readArrayDimensions}
    std::vector<uint32_t> readArrayDimensions() {
        return services::readArrayDimensions(connection_, id_).value();
    }

    /// @wrapper{services::readAccessLevel}
    Bitmask<AccessLevel> readAccessLevel() {
        return services::readAccessLevel(connection_, id_).value();
    }

    /// @wrapper{services::readUserAccessLevel}
    Bitmask<AccessLevel> readUserAccessLevel() {
        return services::readUserAccessLevel(connection_, id_).value();
    }

    /// @wrapper{services::readMinimumSamplingInterval}
    double readMinimumSamplingInterval() {
        return services::readMinimumSamplingInterval(connection_, id_).value();
    }

    /// @wrapper{services::readHistorizing}
    bool readHistorizing() {
        return services::readHistorizing(connection_, id_).value();
    }

    /// @wrapper{services::readExecutable}
    bool readExecutable() {
        return services::readExecutable(connection_, id_).value();
    }

    /// @wrapper{services::readUserExecutable}
    bool readUserExecutable() {
        return services::readUserExecutable(connection_, id_).value();
    }

    /// Read the value of an object property.
    /// @param propertyName Browse name of the property (variable node)
    Variant readObjectProperty(const QualifiedName& propertyName) {
        return browseObjectProperty(propertyName).readValue();
    }

    /// @wrapper{services::writeDisplayName}
    Node& writeDisplayName(const LocalizedText& name) {
        services::writeDisplayName(connection_, id_, name).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeDescription}
    Node& writeDescription(const LocalizedText& desc) {
        services::writeDescription(connection_, id_, desc).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeWriteMask}
    Node& writeWriteMask(Bitmask<WriteMask> mask) {
        services::writeWriteMask(connection_, id_, mask).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeWriteMask}
    Node& writeUserWriteMask(Bitmask<WriteMask> mask) {
        services::writeUserWriteMask(connection_, id_, mask).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeIsAbstract}
    Node& writeIsAbstract(bool isAbstract) {
        services::writeIsAbstract(connection_, id_, isAbstract).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeSymmetric}
    Node& writeSymmetric(bool symmetric) {
        services::writeSymmetric(connection_, id_, symmetric).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeInverseName}
    Node& writeInverseName(const LocalizedText& name) {
        services::writeInverseName(connection_, id_, name).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeContainsNoLoops}
    Node& writeContainsNoLoops(bool containsNoLoops) {
        services::writeContainsNoLoops(connection_, id_, containsNoLoops).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeEventNotifier}
    Node& writeEventNotifier(Bitmask<EventNotifier> mask) {
        services::writeEventNotifier(connection_, id_, mask).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeDataValue}
    Node& writeDataValue(const DataValue& value) {
        services::writeDataValue(connection_, id_, value).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeValue}
    Node& writeValue(const Variant& value) {
        services::writeValue(connection_, id_, value).code().throwIfBad();
        return *this;
    }

    /// Write scalar to variable node.
    template <typename T>
    Node& writeValueScalar(const T& value) {
        // NOLINTNEXTLINE, variant isn't modified, try to avoid copy
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
        services::writeDataType(connection_, id_, typeId).code().throwIfBad();
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
        services::writeValueRank(connection_, id_, valueRank).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeArrayDimensions}
    Node& writeArrayDimensions(Span<const uint32_t> dimensions) {
        services::writeArrayDimensions(connection_, id_, dimensions).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeAccessLevel}
    Node& writeAccessLevel(Bitmask<AccessLevel> mask) {
        services::writeAccessLevel(connection_, id_, mask).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeUserAccessLevel}
    Node& writeUserAccessLevel(Bitmask<AccessLevel> mask) {
        services::writeUserAccessLevel(connection_, id_, mask).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeMinimumSamplingInterval}
    Node& writeMinimumSamplingInterval(double milliseconds) {
        services::writeMinimumSamplingInterval(connection_, id_, milliseconds).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeHistorizing}
    Node& writeHistorizing(bool historizing) {
        services::writeHistorizing(connection_, id_, historizing).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeExecutable}
    Node& writeExecutable(bool executable) {
        services::writeExecutable(connection_, id_, executable).code().throwIfBad();
        return *this;
    }

    /// @wrapper{services::writeUserExecutable}
    Node& writeUserExecutable(bool userExecutable) {
        services::writeUserExecutable(connection_, id_, userExecutable).code().throwIfBad();
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
    Node browseObjectProperty(const QualifiedName& propertyName) {
        auto result =
            services::translateBrowsePathToNodeIds(
                connection_,
                BrowsePath(id_, {{ReferenceTypeId::HasProperty, false, true, propertyName}})
            ).value();
        result.getStatusCode().throwIfBad();
        for (auto&& target : result.getTargets()) {
            if (target.getTargetId().isLocal()) {
                return {connection_, std::move(target.getTargetId().getNodeId())};
            }
        }
        throw BadStatus(UA_STATUSCODE_BADNOTFOUND);
    }

    Connection& connection_;
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
