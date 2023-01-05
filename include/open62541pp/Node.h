#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Server.h"
#include "open62541pp/NodeId.h"
#include "open62541pp/Variant.h"
#include "open62541pp/Types.h"

namespace opcua {

class Node {
public:
    Node(const Server& server, const NodeId& id);

    inline NodeId getNodeId() const { return nodeId_; }

    // getParentNode?
    // getChildNodes?

    NodeClass   getNodeClass();
    std::string getBrowseName();
    std::string getDisplayName();
    std::string getDescription();
    uint32_t    getWriteMask();
    // uint32_t    getUserWriteMask();

    /// Get data type of variable (type) node as NodeId
    NodeId getDataType();
    /// Get access level mask of variable node, e.g. ::UA_ACCESSLEVELMASK_READ | ::UA_ACCESSLEVELMASK_WRITE
    uint8_t getAccessLevel();

    // writeBrowseName disabled for performance reasons:
    // https://github.com/open62541/open62541/issues/3545
    // void setBrowseName(std::string_view name);
    void setDisplayName(std::string_view name, std::string_view locale = "en");
    void setDescription(std::string_view name, std::string_view locale = "en");
    void setWriteMask(uint32_t mask);
    // void setUserWriteMask(uint32_t mask);

    /// Set data type of variable (type) node
    void setDataType(Type type);
    /// Set data type of variable (type) node by NodeId
    void setDataType(const NodeId& typeId);
    /// Set access level mask of variable node, e.g. ::UA_ACCESSLEVELMASK_READ | ::UA_ACCESSLEVELMASK_WRITE
    void setAccessLevel(uint8_t mask);

    Node addFolder(const NodeId& id,       std::string_view browseName);
    Node addObject(const NodeId& id,       std::string_view browseName);
    Node addVariable(const NodeId& id,     std::string_view browseName, Type type);
    Node addProperty(const NodeId& id,     std::string_view browseName, Type type);
    Node addObjectType(const NodeId& id,   std::string_view browseName);
    Node addVariableType(const NodeId& id, std::string_view browseName, Type type);

    /*
     * VariableNode specific methods
     * get/set
     * Value -> read / write
     * DataType
     * ValueRank
     * ArrayDimensions
     * AccessLevel
     * MinimumSamplingFrequency
     * Historizing
     */

    template <typename Arg> // perfect forwarding
    void write(Arg&& arg) {
        Variant var(std::forward<Arg>(arg));
        writeVariantToServer(var);
    }

    template <typename Arg> // perfect forwarding
    void writeArray(Arg&& arg) {
        requireNodeClass(NodeClass::Variable);
        Variant var(std::forward<Arg>(arg));
        writeVariantToServer(var);
    }

    template <typename T>
    T read() {
        Variant var;
        readVariantFromServer(var);
        return var.readScalar<T>();
    }

    template <typename T>
    std::vector<T> readArray() {
        requireNodeClass(NodeClass::Variable);
        Variant var;
        readVariantFromServer(var);
        return var.readArray<T>();
    }

    void remove();

protected:
    Server    server_;
    NodeId    nodeId_;
    NodeClass nodeClass_;

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

    void writeVariantToServer(Variant& var); // should be const Variant&
    void readVariantFromServer(Variant& var) noexcept;
};

} // namespace opcua
