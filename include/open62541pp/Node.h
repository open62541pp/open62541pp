#pragma once

#include <cstdint>
#include <string_view>
#include <utility>  // move
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/Config.h"
#include "open62541pp/NodeIds.h"
#include "open62541pp/Span.h"
#include "open62541pp/TypeConverter.h"  // guessDataType
#include "open62541pp/TypeWrapper.h"  // asWrapper
#include "open62541pp/open62541.h"
#include "open62541pp/services/Attribute.h"
#include "open62541pp/services/Method.h"
#include "open62541pp/services/NodeManagement.h"
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
    Node(ServerOrClient& connection, NodeId id)
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
        services::addFolder(connection_, nodeId_, id, browseName, attributes, referenceType);
        return {connection_, id};
    }

    /// @copydoc services::addObject
    Node addObject(
        const NodeId& id,
        std::string_view browseName,
        const ObjectAttributes& attributes = {},
        const NodeId& objectType = ObjectTypeId::BaseObjectType,
        const NodeId& referenceType = ReferenceTypeId::HasComponent
    ) {
        services::addObject(
            connection_, nodeId_, id, browseName, attributes, objectType, referenceType
        );
        return {connection_, id};
    }

    /// @copydoc services::addVariable
    Node addVariable(
        const NodeId& id,
        std::string_view browseName,
        const VariableAttributes& attributes = {},
        const NodeId& variableType = VariableTypeId::BaseDataVariableType,
        const NodeId& referenceType = ReferenceTypeId::HasComponent
    ) {
        services::addVariable(
            connection_, nodeId_, id, browseName, attributes, variableType, referenceType
        );
        return {connection_, id};
    }

    /// @copydoc services::addProperty
    Node addProperty(
        const NodeId& id, std::string_view browseName, const VariableAttributes& attributes = {}
    ) {
        services::addProperty(connection_, nodeId_, id, browseName, attributes);
        return {connection_, id};
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
        services::addMethod(
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
        return {connection_, id};
    }
#endif

    /// @copydoc services::addObjectType
    Node addObjectType(
        const NodeId& id,
        std::string_view browseName,
        const ObjectTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        services::addObjectType(connection_, nodeId_, id, browseName, attributes, referenceType);
        return {connection_, id};
    }

    /// @copydoc services::addVariableType
    Node addVariableType(
        const NodeId& id,
        std::string_view browseName,
        const VariableTypeAttributes& attributes = {},
        const NodeId& variableType = VariableTypeId::BaseDataVariableType,
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        services::addVariableType(
            connection_, nodeId_, id, browseName, attributes, variableType, referenceType
        );
        return {connection_, id};
    }

    /// @copydoc services::addReferenceType
    Node addReferenceType(
        const NodeId& id,
        std::string_view browseName,
        const ReferenceTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        services::addReferenceType(connection_, nodeId_, id, browseName, attributes, referenceType);
        return {connection_, id};
    }

    /// @copydoc services::addDataType
    Node addDataType(
        const NodeId& id,
        std::string_view browseName,
        const DataTypeAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::HasSubtype
    ) {
        services::addDataType(connection_, nodeId_, id, browseName, attributes, referenceType);
        return {connection_, id};
    }

    /// @copydoc services::addView
    Node addView(
        const NodeId& id,
        std::string_view browseName,
        const ViewAttributes& attributes = {},
        const NodeId& referenceType = ReferenceTypeId::Organizes
    ) {
        services::addView(connection_, nodeId_, id, browseName, attributes, referenceType);
        return {connection_, id};
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
        uint32_t nodeClassMask = UA_NODECLASS_UNSPECIFIED
    );

    /// Browse referenced nodes (only local nodes).
    std::vector<Node> browseReferencedNodes(
        BrowseDirection browseDirection = BrowseDirection::Both,
        const NodeId& referenceType = ReferenceTypeId::References,
        bool includeSubtypes = true,
        uint32_t nodeClassMask = UA_NODECLASS_UNSPECIFIED
    );

    /// Browse child nodes (only local nodes).
    std::vector<Node> browseChildren(
        const NodeId& referenceType = ReferenceTypeId::HierarchicalReferences,
        uint32_t nodeClassMask = UA_NODECLASS_UNSPECIFIED
    ) {
        return browseReferencedNodes(BrowseDirection::Forward, referenceType, true, nodeClassMask);
    }

    /// Browse child node specified by its relative path from this node (only local nodes).
    /// The relative path is specified using browse names.
    /// @exception BadStatus (BadNoMatch) If path not found
    Node browseChild(Span<const QualifiedName> path);

    /// Browse parent node.
    /// A Node may have several parents, the first found is returned.
    /// @exception BadStatus (BadNotFound) If no parent node found
    Node browseParent();

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
    uint32_t readWriteMask() {
        return services::readWriteMask(connection_, nodeId_);
    }

    /// @copydoc services::readUserWriteMask
    uint32_t readUserWriteMask() {
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

    /// @copydoc services::readDataValue
    DataValue readDataValue() {
        return services::readDataValue(connection_, nodeId_);
    }

    /// @copydoc services::readDataValue
    [[deprecated("No performance benefit to pass DataValue by reference, return by value instead"
    )]] void
    readDataValue(DataValue& value) {
        value = services::readDataValue(connection_, nodeId_);
    }

    /// @copydoc services::readValue
    Variant readValue() {
        return services::readValue(connection_, nodeId_);
    }

    /// @copydoc services::readValue
    [[deprecated("No performance benefit to pass Variant by reference, return by value instead"
    )]] void
    readValue(Variant& value) {
        value = services::readValue(connection_, nodeId_);
    }

    /// Read scalar value from variable node.
    template <typename T>
    T readValueScalar() {
        return readValue().template getScalarCopy<T>();
    }

    /// @copydoc readValueScalar
    template <typename T>
    [[deprecated("Use Node::readValueScalar instead")]] T readScalar() {
        return readValueScalar<T>();
    }

    /// Read array value from variable node.
    template <typename T>
    std::vector<T> readValueArray() {
        return readValue().template getArrayCopy<T>();
    }

    /// @copydoc readValueArray
    template <typename T>
    [[deprecated("Use Node::readValueArray instead")]] std::vector<T> readArray() {
        return readValueArray<T>();
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
    uint8_t readAccessLevel() {
        return services::readAccessLevel(connection_, nodeId_);
    }

    /// @copydoc services::readUserAccessLevel
    uint8_t readUserAccessLevel() {
        return services::readUserAccessLevel(connection_, nodeId_);
    }

    /// @copydoc services::readMinimumSamplingInterval
    double readMinimumSamplingInterval() {
        return services::readMinimumSamplingInterval(connection_, nodeId_);
    }

    /// Read the value of an object property.
    /// @param propertyName Browse name of the property (variable node)
    Variant readObjectProperty(const QualifiedName& propertyName);

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
    Node& writeWriteMask(uint32_t mask) {
        services::writeWriteMask(connection_, nodeId_, mask);
        return *this;
    }

    /// @copydoc services::writeWriteMask
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeUserWriteMask(uint32_t mask) {
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
        const auto variant = Variant::fromScalar<T>(const_cast<T&>(value));
        writeValue(variant);
        return *this;
    }

    /// @copydoc writeValueScalar
    template <typename T>
    [[deprecated("Use Node::writeValueScalar instead")]] Node& writeScalar(const T& value) {
        return writeValueScalar<T>(value);
    }

    /// Write array value to variable node.
    /// @return Current node instance to chain multiple methods (fluent interface)
    template <typename T>
    Node& writeValueArray(Span<T> array) {
        writeValue(Variant::fromArray(array));
        return *this;
    }

    /// @overload
    template <typename ArrayLike>
    Node& writeValueArray(ArrayLike&& array) {
        return writeValueArray(Span{std::forward<ArrayLike>(array)});
    }

    /// Write range of elements as array value to variable node.
    /// @return Current node instance to chain multiple methods (fluent interface)
    template <typename InputIt>
    Node& writeValueArray(InputIt first, InputIt last) {
        writeValue(Variant::fromArray(first, last));
        return *this;
    }

    /// @copydoc  writeValueArray
    template <typename... Args>
    [[deprecated("Use Node::writeValueArray instead")]] Node& writeArray(Args&&... args) {
        return writeValueArray(std::forward<Args>(args)...);
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
        return writeDataType(asWrapper<NodeId>(detail::guessDataType<T>().typeId));
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
    Node& writeAccessLevel(uint8_t mask) {
        services::writeAccessLevel(connection_, nodeId_, mask);
        return *this;
    }

    /// @copydoc services::writeUserAccessLevel
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeUserAccessLevel(uint8_t mask) {
        services::writeUserAccessLevel(connection_, nodeId_, mask);
        return *this;
    }

    /// @copydoc services::writeMinimumSamplingInterval
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeMinimumSamplingInterval(double milliseconds) {
        services::writeMinimumSamplingInterval(connection_, nodeId_, milliseconds);
        return *this;
    }

    /// Write the value of an object property.
    /// @param propertyName Browse name of the property (variable node)
    /// @param value New value
    /// @return Current node instance to chain multiple methods (fluent interface)
    Node& writeObjectProperty(const QualifiedName& propertyName, const Variant& value);

private:
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
