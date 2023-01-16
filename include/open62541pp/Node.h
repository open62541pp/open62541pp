#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>  // forward
#include <vector>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/NodeId.h"
#include "open62541pp/Server.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/Types.h"
#include "open62541pp/Variant.h"

namespace opcua {

/**
 * High level node object, to access node attribute, browse and populate address space.
 *
 * Node objects are usefull as-is but they do not expose the entire OPC UA protocol.
 * You can get access to the associated NodeId instance with the getNodeId() method and apply the
 * native open62541 functions.
 */
class Node {
public:
    Node(const Server& server, const NodeId& id);

    // getParentNode?
    // getChildNodes?

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

    Node addFolder(const NodeId& id, std::string_view browseName);
    Node addObject(const NodeId& id, std::string_view browseName);
    Node addVariable(const NodeId& id, std::string_view browseName, Type type);
    Node addProperty(const NodeId& id, std::string_view browseName, Type type);
    Node addObjectType(const NodeId& id, std::string_view browseName);
    Node addVariableType(const NodeId& id, std::string_view browseName, Type type);

    template <typename Arg>  // perfect forwarding
    void write(Arg&& arg) {
        requireNodeClass(NodeClass::Variable);
        Variant var;
        var.setScalarCopy(std::forward<Arg>(arg));
        writeVariantToServer(var);
    }

    template <typename Arg>  // perfect forwarding
    void writeArray(Arg&& arg) {
        requireNodeClass(NodeClass::Variable);
        Variant var;
        var.setArrayCopy(std::forward<Arg>(arg));
        writeVariantToServer(var);
    }

    template <typename T>
    T read() {
        requireNodeClass(NodeClass::Variable);
        Variant var;
        readVariantFromServer(var);
        return var.getScalarCopy<T>();
    }

    template <typename T>
    std::vector<T> readArray() {
        requireNodeClass(NodeClass::Variable);
        Variant var;
        readVariantFromServer(var);
        return var.getArrayCopy<T>();
    }

    void remove();

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

    void writeVariantToServer(Variant& var);  // should be const Variant&
    void readVariantFromServer(Variant& var) noexcept;

private:
    Server server_;
    NodeId nodeId_;
    NodeClass nodeClass_;
};

}  // namespace opcua
