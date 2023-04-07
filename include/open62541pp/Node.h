#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "open62541pp/Common.h"
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

    /// @copydoc services::addFolder
    Node addFolder(
        const NodeId& id,
        std::string_view browseName,
        ReferenceType referenceType = ReferenceType::HasComponent
    );

    /// @copydoc services::addObject
    Node addObject(
        const NodeId& id,
        std::string_view browseName,
        const NodeId& objectType = {0, UA_NS0ID_BASEOBJECTTYPE},
        ReferenceType referenceType = ReferenceType::HasComponent
    );

    /// @copydoc services::addVariable
    Node addVariable(
        const NodeId& id,
        std::string_view browseName,
        const NodeId& variableType = {0, UA_NS0ID_BASEDATAVARIABLETYPE},
        ReferenceType referenceType = ReferenceType::HasComponent
    );

    /// @copydoc services::addProperty
    Node addProperty(const NodeId& id, std::string_view browseName);

    /// @copydoc services::addObjectType
    Node addObjectType(
        const NodeId& id,
        std::string_view browseName,
        ReferenceType referenceType = ReferenceType::HasSubType
    );

    /// @copydoc services::addVariableType
    Node addVariableType(
        const NodeId& id,
        std::string_view browseName,
        const NodeId& variableType = {0, UA_NS0ID_BASEDATAVARIABLETYPE},
        ReferenceType referenceType = ReferenceType::HasSubType
    );

    /// @copydoc services::addReference
    void addReference(const NodeId& targetId, ReferenceType referenceType, bool forward = true);

    /// @copydoc services::deleteNode
    void deleteNode(bool deleteReferences = true);

    /// Get a child specified by its path from this node (only local nodes).
    /// @exception BadStatus If path not found (BadNoMatch)
    Node getChild(const std::vector<QualifiedName>& path);

    /// @copydoc services::readNodeClass
    NodeClass readNodeClass();

    /// @copydoc services::readBrowseName
    std::string readBrowseName();

    /// @copydoc services::readDisplayName
    LocalizedText readDisplayName();

    /// @copydoc services::readDescription
    LocalizedText readDescription();

    /// @copydoc services::readWriteMask
    uint32_t readWriteMask();

    /// @copydoc services::readDataType
    NodeId readDataType();

    /// @copydoc services::readValueRank
    ValueRank readValueRank();

    /// @copydoc services::readArrayDimensions
    std::vector<uint32_t> readArrayDimensions();

    /// @copydoc services::readAccessLevel
    uint8_t readAccessLevel();

    /// @copydoc services::readDataValue
    void readDataValue(DataValue& value);

    /// @copydoc services::readValue
    void readValue(Variant& value);

    /// Read scalar from variable node.
    template <typename T>
    T readScalar();

    /// Read array from variable node.
    template <typename T>
    std::vector<T> readArray();

    /// @copydoc services::writeDisplayName
    void writeDisplayName(std::string_view name, std::string_view locale);

    /// @copydoc services::writeDescription
    void writeDescription(std::string_view name, std::string_view locale);

    /// @copydoc services::writeWriteMask
    void writeWriteMask(uint32_t mask);

    /// @copydoc services::writeDataType(Server&, const NodeId&, Type)
    void writeDataType(Type type);

    /// @copydoc services::writeDataType(Server&, const NodeId&, const NodeId&)
    void writeDataType(const NodeId& typeId);

    /// @copydoc services::writeValueRank
    void writeValueRank(ValueRank valueRank);

    /// @copydoc services::writeArrayDimensions
    void writeArrayDimensions(const std::vector<uint32_t>& dimensions);

    /// @copydoc services::writeAccessLevel
    void writeAccessLevel(uint8_t mask);

    /// @copydoc services::writeModellingRule
    void writeModellingRule(ModellingRule rule);

    /// @copydoc services::writeDataValue
    void writeDataValue(const DataValue& value);

    /// @copydoc services::writeValue
    void writeValue(const Variant& value);

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

private:
    Server server_;
    NodeId nodeId_;
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
    // NOLINTNEXTLINE, variant isn't modified, try to avoid copy
    const auto variant = Variant::fromScalar<T, type>(const_cast<T&>(value));
    writeValue(variant);
}

template <typename T, Type type>
void Node::writeArray(const T* array, size_t size) {
    // NOLINTNEXTLINE, variant isn't modified, try to avoid copy
    const auto variant = Variant::fromArray<T, type>(const_cast<T*>(array), size);
    writeValue(variant);
}

template <typename T, Type type>
void Node::writeArray(const std::vector<T>& array) {
    writeArray<T, type>(array.data(), array.size());
}

template <typename InputIt, Type type>
void Node::writeArray(InputIt first, InputIt last) {
    const auto variant = Variant::fromArray<InputIt, type>(first, last);
    writeValue(variant);
}

/* ---------------------------------------------------------------------------------------------- */

bool operator==(const Node& left, const Node& right) noexcept;
bool operator!=(const Node& left, const Node& right) noexcept;

}  // namespace opcua
