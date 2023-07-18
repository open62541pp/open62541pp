#pragma once

#include <cstdint>
#include <string_view>
#include <utility>  // move
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/Config.h"
#include "open62541pp/TypeConverter.h"  // guessType
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
 * High-level node object to access node attribute, browse and populate address space.
 *
 * The Node API is just a more convenient way of using the free functions in the `services`
 * namespace.
 *
 * Node objects are usefull as-is but they do not expose the entire OPC UA protocol. You can get
 * access to the associated NodeId instance with the getNodeId() method and apply the native
 * open62541 functions or the free functions in the `services` namespace.
 *
 * @see Services
 */
template <typename ServerOrClient>
class Node {
public:
    /// Create a Node object.
    /// @exception BadStatus (BadNodeIdUnknown) If `checkExists` enabled and `id` not found
    Node(ServerOrClient connection, NodeId id, bool checkExists = true)
        : connection_(std::move(connection)),
          nodeId_(std::move(id)) {
        if (checkExists) {
            services::readNodeId(connection_, nodeId_);
        }
    }

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

    /// @copydoc services::addFolder
    Node addFolder(
        const NodeId& id,
        std::string_view browseName,
        const NodeId& referenceType = ReferenceTypeId::HasComponent,
        const ObjectAttributes& attributes = {}
    ) {
        services::addFolder(connection_, nodeId_, id, browseName, referenceType, attributes);
        return {connection_, id, false};
    }

    /// @copydoc services::addObject
    Node addObject(
        const NodeId& id,
        std::string_view browseName,
        const NodeId& objectType = ObjectTypeId::BaseObjectType,
        const NodeId& referenceType = ReferenceTypeId::HasComponent,
        const ObjectAttributes& attributes = {}
    ) {
        services::addObject(
            connection_, nodeId_, id, browseName, objectType, referenceType, attributes
        );
        return {connection_, id, false};
    }

    /// @copydoc services::addVariable
    Node addVariable(
        const NodeId& id,
        std::string_view browseName,
        const NodeId& variableType = VariableTypeId::BaseDataVariableType,
        const NodeId& referenceType = ReferenceTypeId::HasComponent,
        const VariableAttributes& attributes = {}
    ) {
        services::addVariable(
            connection_, nodeId_, id, browseName, variableType, referenceType, attributes
        );
        return {connection_, id, false};
    }

    /// @copydoc services::addProperty
    Node addProperty(
        const NodeId& id, std::string_view browseName, const VariableAttributes& attributes = {}
    ) {
        services::addProperty(connection_, nodeId_, id, browseName, attributes);
        return {connection_, id, false};
    }

    /// @copydoc services::addObjectType
    Node addObjectType(
        const NodeId& id,
        std::string_view browseName,
        const NodeId& referenceType = ReferenceTypeId::HasSubtype,
        const ObjectTypeAttributes& attributes = {}
    ) {
        services::addObjectType(connection_, nodeId_, id, browseName, referenceType, attributes);
        return {connection_, id, false};
    }

    /// @copydoc services::addVariableType
    Node addVariableType(
        const NodeId& id,
        std::string_view browseName,
        const NodeId& variableType = VariableTypeId::BaseDataVariableType,
        const NodeId& referenceType = ReferenceTypeId::HasSubtype,
        const VariableTypeAttributes& attributes = {}
    ) {
        services::addVariableType(
            connection_, nodeId_, id, browseName, variableType, referenceType, attributes
        );
        return {connection_, id, false};
    }

#ifdef UA_ENABLE_METHODCALLS
    /// @copydoc services::addMethod
    Node addMethod(
        const NodeId& id,
        std::string_view browseName,
        services::MethodCallback callback,
        const std::vector<Argument>& inputArguments,
        const std::vector<Argument>& outputArguments,
        const NodeId& referenceType = ReferenceTypeId::HasComponent,
        const MethodAttributes& attributes = {}
    ) {
        services::addMethod(
            connection_,
            nodeId_,
            id,
            browseName,
            std::move(callback),
            inputArguments,
            outputArguments,
            referenceType,
            attributes
        );
        return {connection_, id, false};
    }
#endif

    /// @copydoc services::addReference
    void addReference(const NodeId& targetId, const NodeId& referenceType, bool forward = true) {
        services::addReference(connection_, nodeId_, targetId, referenceType, forward);
    }

    /// @copydoc services::addModellingRule
    void addModellingRule(ModellingRule rule) {
        services::addModellingRule(connection_, nodeId_, rule);
    }

    /// @copydoc services::deleteNode
    void deleteNode(bool deleteReferences = true) {
        services::deleteNode(connection_, nodeId_, deleteReferences);
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
    Node browseChild(const std::vector<QualifiedName>& path);

    /// Browse parent node.
    /// A Node may have several parents, the first found is returned.
    /// @exception BadStatus (BadNotFound) If no parent node found
    Node browseParent();

#ifdef UA_ENABLE_METHODCALLS
    /// Call a server method and return results.
    /// @param methodId NodeId of the method (`HasComponent` reference to current node required)
    /// @param inputArguments Input argument values
    std::vector<Variant> callMethod(
        const NodeId& methodId, const std::vector<Variant>& inputArguments
    ) {
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
    void readDataValue(DataValue& value) {
        services::readDataValue(connection_, nodeId_, value);
    }

    /// @copydoc services::readValue
    void readValue(Variant& value) {
        services::readValue(connection_, nodeId_, value);
    }

    /// Read scalar from variable node.
    template <typename T>
    T readScalar() {
        Variant variant;
        readValue(variant);
        return variant.getScalarCopy<T>();
    }

    /// Read array from variable node.
    template <typename T>
    std::vector<T> readArray() {
        Variant variant;
        readValue(variant);
        return variant.getArrayCopy<T>();
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

    /// @copydoc services::writeDisplayName
    void writeDisplayName(const LocalizedText& name) {
        services::writeDisplayName(connection_, nodeId_, name);
    }

    /// @copydoc services::writeDescription
    void writeDescription(const LocalizedText& desc) {
        services::writeDescription(connection_, nodeId_, desc);
    }

    /// @copydoc services::writeWriteMask
    void writeWriteMask(uint32_t mask) {
        services::writeWriteMask(connection_, nodeId_, mask);
    }

    /// @copydoc services::writeWriteMask
    void writeUserWriteMask(uint32_t mask) {
        services::writeUserWriteMask(connection_, nodeId_, mask);
    }

    /// @copydoc services::writeIsAbstract
    void writeIsAbstract(bool isAbstract) {
        services::writeIsAbstract(connection_, nodeId_, isAbstract);
    }

    /// @copydoc services::writeSymmetric
    void writeSymmetric(bool symmetric) {
        services::writeSymmetric(connection_, nodeId_, symmetric);
    }

    /// @copydoc services::writeInverseName
    void writeInverseName(const LocalizedText& name) {
        services::writeInverseName(connection_, nodeId_, name);
    }

    /// @copydoc services::writeDataValue
    void writeDataValue(const DataValue& value) {
        services::writeDataValue(connection_, nodeId_, value);
    }

    /// @copydoc services::writeValue
    void writeValue(const Variant& value) {
        services::writeValue(connection_, nodeId_, value);
    }

    /// Write scalar to variable node.
    template <typename T, Type type = detail::guessType<T>()>
    void writeScalar(const T& value) {
        // NOLINTNEXTLINE, variant isn't modified, try to avoid copy
        const auto variant = Variant::fromScalar<T, type>(const_cast<T&>(value));
        writeValue(variant);
    }

    /// Write array (raw) to variable node.
    template <typename T, Type type = detail::guessType<T>()>
    void writeArray(const T* array, size_t size) {
        // NOLINTNEXTLINE, variant isn't modified, try to avoid copy
        const auto variant = Variant::fromArray<T, type>(const_cast<T*>(array), size);
        writeValue(variant);
    }

    /// Write array (std::vector) to variable node.
    template <typename T, Type type = detail::guessType<T>()>
    void writeArray(const std::vector<T>& array) {
        writeArray<T, type>(array.data(), array.size());
    }

    /// Write range of elements as array to variable node.
    template <typename InputIt, Type type = detail::guessTypeFromIterator<InputIt>()>
    void writeArray(InputIt first, InputIt last) {
        const auto variant = Variant::fromArray<InputIt, type>(first, last);
        writeValue(variant);
    }

    /// @copydoc services::writeDataType(T&, const NodeId&, Type)
    void writeDataType(Type type) {
        services::writeDataType(connection_, nodeId_, type);
    }

    /// @copydoc services::writeDataType(T&, const NodeId&, const NodeId&)
    void writeDataType(const NodeId& typeId) {
        services::writeDataType(connection_, nodeId_, typeId);
    }

    /// @copydoc services::writeValueRank
    void writeValueRank(ValueRank valueRank) {
        services::writeValueRank(connection_, nodeId_, valueRank);
    }

    /// @copydoc services::writeArrayDimensions
    void writeArrayDimensions(const std::vector<uint32_t>& dimensions) {
        services::writeArrayDimensions(connection_, nodeId_, dimensions);
    }

    /// @copydoc services::writeAccessLevel
    void writeAccessLevel(uint8_t mask) {
        services::writeAccessLevel(connection_, nodeId_, mask);
    }

    /// @copydoc services::writeUserAccessLevel
    void writeUserAccessLevel(uint8_t mask) {
        services::writeUserAccessLevel(connection_, nodeId_, mask);
    }

    /// @copydoc services::writeMinimumSamplingInterval
    void writeMinimumSamplingInterval(double milliseconds) {
        services::writeMinimumSamplingInterval(connection_, nodeId_, milliseconds);
    }

private:
    ServerOrClient connection_;
    NodeId nodeId_;
};

/* ---------------------------------------------------------------------------------------------- */

template <typename ServerOrClient>
bool operator==(const Node<ServerOrClient>& left, const Node<ServerOrClient>& right) noexcept {
    return (left.getConnection() == right.getConnection()) &&
           (left.getNodeId() == right.getNodeId());
}

template <typename ServerOrClient>
bool operator!=(const Node<ServerOrClient>& left, const Node<ServerOrClient>& right) noexcept {
    return !(left == right);
}

}  // namespace opcua
