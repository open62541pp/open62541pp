#pragma once

#include <string_view>

#include "open62541/types.h"
#include "open62541/types_generated_handling.h"

namespace opcua {

class NodeId {
public:
    NodeId(uint32_t identifier, uint16_t namespaceIndex = 0)
        : id_(UA_NODEID_NUMERIC(namespaceIndex, identifier)) {}

    NodeId(std::string_view identifier, uint16_t namespaceIndex = 0)
        : id_(UA_NODEID_STRING_ALLOC(namespaceIndex, identifier.data())) {}

    NodeId(const UA_NodeId& id) {
        UA_NodeId_copy(&id, &id_);
    }

    ~NodeId() { UA_NodeId_deleteMembers(&id_); }

    NodeId(const NodeId& other)            { UA_NodeId_copy(&other.id_, &id_); }
    NodeId& operator=(const NodeId& other) { UA_NodeId_copy(&other.id_, &id_); return *this; }

    // NodeId(NodeId&& other) = default;
    // NodeId& operator=(NodeId&& other) = default;

    // GUID and BYTESTRING not implemented

    inline bool operator==(const NodeId& other) const { return UA_NodeId_equal(&id_, other.handle()); }
    inline bool operator!=(const NodeId& other) const { return !operator==(other); }

    inline uint32_t hash() const { return UA_NodeId_hash(&id_); }

    inline auto getNamespaceIndex() const { return id_.namespaceIndex; }

    inline       UA_NodeId* handle()       { return &id_; }
    inline const UA_NodeId* handle() const { return &id_; }
private:
    UA_NodeId id_;
};

} // namespace opc ua
