#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>  // forward
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeConverter.h"  // guessType
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

namespace opcua {

/**
 * High-level node object, to access node attribute, browse and populate address space.
 *
 * Node objects are usefull as-is but they do not expose the entire OPC UA protocol.
 * You can get access to the associated NodeId instance with the getNodeId() method and apply the
 * native open62541 functions.
 */
class Node {
public:
    Node(const Server& server, const NodeId& id);

    /// Get server instance.
    Server& getServer() noexcept;

    /// Get server instance.
    const Server& getServer() const noexcept;

    /// Get node id.
    const NodeId& getNodeId() const noexcept;

    /// Get node class.
    NodeClass getNodeClass() const noexcept;

    /// Get browse name.
    std::string getBrowseName();
    /// Get localized display name.
    LocalizedText getDisplayName();
    /// Get localized description.
    LocalizedText getDescription();
    /// Get write mask, e.g. `::UA_WRITEMASK_ACCESSLEVEL | ::UA_WRITEMASK_DESCRIPTION`.
    uint32_t getWriteMask();
    // uint32_t getUserWriteMask();

    /// Get data type of variable (type) node as NodeId.
    NodeId getDataType();
    /// Get value rank of variable (type) node.
    ValueRank getValueRank();
    /// Get array dimensions of variable (type) node.
    std::vector<uint32_t> getArrayDimensions();
    /// Get access level mask of variable node, e.g. `::UA_ACCESSLEVELMASK_READ`.
    uint8_t getAccessLevel();

    // writeBrowseName disabled for performance reasons:
    // https://github.com/open62541/open62541/issues/3545
    // void setBrowseName(std::string_view name);

    /// Set localized display name.
    void setDisplayName(std::string_view name, std::string_view locale);
    /// Set localized description.
    void setDescription(std::string_view name, std::string_view locale);
    /// Set write mask, e.g. `::UA_WRITEMASK_ACCESSLEVEL | ::UA_WRITEMASK_DESCRIPTION`.
    void setWriteMask(uint32_t mask);
    // void setUserWriteMask(uint32_t mask);

    /// Set data type of variable (type) node.
    void setDataType(Type type);
    /// Set data type of variable (type) node by node id.
    void setDataType(const NodeId& typeId);
    /// Set value rank of variable (type) node.
    void setValueRank(ValueRank valueRank);
    /// Set array dimensions of variable (type) node.
    /// Should be unspecified if ValueRank is <= 0 (ValueRank::Any, ValueRank::Scalar,
    /// ValueRank::ScalarOrOneDimension, ValueRank::OneOrMoreDimensions). The dimension zero is a
    /// wildcard and the actual value may have any length in this dimension.
    void setArrayDimensions(const std::vector<uint32_t>& dimensions);
    /// Set access level mask of variable node,
    /// e.g. `::UA_ACCESSLEVELMASK_READ | ::UA_ACCESSLEVELMASK_WRITE`.
    void setAccessLevel(uint8_t mask);
    /// Set modelling rule.
    void setModellingRule(ModellingRule rule);

    /// Add child folder to node.
    Node addFolder(
        const NodeId& id,
        std::string_view browseName,
        ReferenceType referenceType = ReferenceType::HasComponent
    );

    /// Add child object to node.
    Node addObject(
        const NodeId& id,
        std::string_view browseName,
        const NodeId& objectType = {0, UA_NS0ID_BASEOBJECTTYPE},
        ReferenceType referenceType = ReferenceType::HasComponent
    );

    /// Add child variable to node.
    Node addVariable(
        const NodeId& id,
        std::string_view browseName,
        const NodeId& variableType = {0, UA_NS0ID_BASEDATAVARIABLETYPE},
        ReferenceType referenceType = ReferenceType::HasComponent
    );

    /// Add child property to node.
    Node addProperty(const NodeId& id, std::string_view browseName);

    /// Add child object type to node.
    Node addObjectType(
        const NodeId& id,
        std::string_view browseName,
        ReferenceType referenceType = ReferenceType::HasSubType
    );

    /// Add child variable type to node.
    Node addVariableType(
        const NodeId& id,
        std::string_view browseName,
        const NodeId& variableType = {0, UA_NS0ID_BASEDATAVARIABLETYPE},
        ReferenceType referenceType = ReferenceType::HasSubType
    );

    /// Add reference.
    void addReference(const NodeId& target, ReferenceType referenceType, bool forward = true);

    /// Get a child specified by its path from this node (only local nodes).
    /// @exception BadStatus If path not found (BadNoMatch)
    Node getChild(const std::vector<QualifiedName>& path);

    /// Read value from variable node as DataValue object.
    void readDataValue(DataValue& value);

    /// Read value from variable node as Variant object.
    void readValue(Variant& variant);

    /// Read scalar from variable node.
    template <typename T>
    T readScalar();

    /// Read array from variable node.
    template <typename T>
    std::vector<T> readArray();

    /// Write DataValue to variable node.
    void writeDataValue(const DataValue& value);

    /// Write Variant to variable node.
    void writeValue(const Variant& variant);

    /// Write scalar to variable node.
    template <typename T, Type type = detail::guessType<T>()>
    void writeScalar(const T& value);

    /// Write array (raw) to variable node.
    template <typename T, Type type = detail::guessType<T>()>
    void writeArray(const T* array, size_t size);

    /// Write array (std::vector) to variable node.
    template <typename T, Type type = detail::guessType<T>()>
    void writeArray(const std::vector<T>& array);

    /// Write range of elements as array to variable node.
    template <typename InputIt, Type type = detail::guessTypeFromIterator<InputIt>()>
    void writeArray(InputIt first, InputIt last);

    void remove(bool deleteReferences = true);

protected:
    template <typename... Ts>
    constexpr bool isNodeClass(Ts&&... classes) {
        const auto isSame = [&](NodeClass c) { return nodeClass_ == c; };
        return (isSame(classes) || ...);
    }

    template <typename... Ts>
    void requireNodeClass(Ts&&... classes) {
        const bool isAnyOf = isNodeClass(std::forward<Ts>(classes)...);
        if (!isAnyOf) {
            const auto nodeClassName = getNodeClassName(nodeClass_);
            throw InvalidNodeClass(
                std::string("Operation not allowed for nodes of class ").append(nodeClassName)
            );
        }
    }

private:
    Server server_;
    NodeId nodeId_;
    NodeClass nodeClass_;
};

/* ---------------------------------------------------------------------------------------------- */

template <typename T>
T Node::readScalar() {
    Variant variant;
    readValue(variant);
    return variant.getScalarCopy<T>();
}

template <typename T>
std::vector<T> Node::readArray() {
    Variant variant;
    readValue(variant);
    return variant.getArrayCopy<T>();
}

template <typename T, Type type>
void Node::writeScalar(const T& value) {
    Variant variant;
    if constexpr (detail::isAssignableToVariantScalar<T>()) {
        variant.setScalar<T, type>(const_cast<T&>(value));  // NOLINT, variant isn't modified
    } else {
        variant.setScalarCopy<T, type>(value);
    }
    writeValue(variant);
}

template <typename T, Type type>
void Node::writeArray(const T* array, size_t size) {
    Variant variant;
    if constexpr (detail::isAssignableToVariantArray<T>()) {
        variant.setArray<T, type>(const_cast<T*>(array), size);  // NOLINT, variant isn't modified
    } else {
        variant.setArrayCopy<T, type>(array, size);
    }
    writeValue(variant);
}

template <typename T, Type type>
void Node::writeArray(const std::vector<T>& array) {
    writeArray<T, type>(array.data(), array.size());
}

template <typename InputIt, Type type>
void Node::writeArray(InputIt first, InputIt last) {
    Variant variant;
    variant.setArrayCopy<InputIt, type>(first, last);
    writeValue(variant);
}

/* ---------------------------------------------------------------------------------------------- */

bool operator==(const Node& left, const Node& right) noexcept;
bool operator!=(const Node& left, const Node& right) noexcept;

}  // namespace opcua
