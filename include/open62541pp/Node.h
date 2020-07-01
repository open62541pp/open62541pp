#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "Server.h"
#include "NodeId.h"
#include "Variant.h"
#include "Types.h"

namespace opcua {

// forward declaration
class ObjectNode;
class VariableNode;
class MethodNode;
class ViewNode;

class DataTypeNode;
class ObjectTypeNode;
class VariableTypeNode;
class ReferenceTypeNode;
class EventTypeNode;


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
    // Type getDataType();

    // writeBrowseName disabled for performance reasons:
    // https://github.com/open62541/open62541/issues/3545
    // void setBrowseName(std::string_view name);
    void setDisplayName(std::string_view name, std::string_view locale = "en");
    void setDescription(std::string_view name, std::string_view locale = "en");
    void setWriteMask(uint32_t mask);
    // void setUserWriteMask(uint32_t mask);
    // void setDataType(Type type);

    ObjectNode       addFolder(const NodeId& id,       std::string_view browseName);
    ObjectNode       addObject(const NodeId& id,       std::string_view browseName);
    VariableNode     addVariable(const NodeId& id,     std::string_view browseName, Type type);
    ObjectTypeNode   addObjectType(const NodeId& id,   std::string_view browseName);
    VariableTypeNode addVariableType(const NodeId& id, std::string_view browseName, Type type);

    void remove();
protected:
    Server server_;
    NodeId nodeId_;
};


class ObjectNode : public Node { 
public:
    using Node::Node; // inherit constructors
};


class VariableNode: public Node {
public:
    using Node::Node; // inherit constructors

    /** get/set
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
        Variant var;
        readVariantFromServer(var);
        return var.readArray<T>();
    }
private:
    void writeVariantToServer(Variant& var); // should be const Variant&
    void readVariantFromServer(Variant& var) noexcept;
};

class MethodeNode : public Node {
public:
    using Node::Node; // inherit constructors
};

class ViewNode : public Node {
public:
    using Node::Node; // inherit constructors
};

class DataTypeNode : public Node { 
public:
    using Node::Node; // inherit constructors
};

class ObjectTypeNode : public Node { 
public:
    using Node::Node; // inherit constructors
};

class VariableTypeNode : public Node { 
public:
    using Node::Node; // inherit constructors
};

class ReferenceTypeNode : public Node { 
public:
    using Node::Node; // inherit constructors
};

} // namespace opcua
