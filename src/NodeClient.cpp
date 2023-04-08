#include "open62541pp/NodeClient.h"

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Helper.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/TypeConverter.h"
using namespace opcua::detail;

#include "open62541/client_highlevel.h"
#include "open62541_impl.h"

namespace opcua {
NodeClient::NodeClient(std::shared_ptr<Client> client, const NodeId& id)  // NOLINT
    : client_(client),
      nodeId_(id) {
    // check if node exists
    {
        NodeId outputNode(UA_NODEID_NULL);
        const auto status = UA_Client_readNodeIdAttribute(client_->handle(), nodeId_, outputNode.handle());
        if (status!= UA_STATUSCODE_GOOD)
            throw std::runtime_error("Failed to verify node existence");
    }
//    // store node class
    {
        UA_NodeClass nodeClass = UA_NODECLASS_UNSPECIFIED;
        const auto status = UA_Client_readNodeClassAttribute(client_->handle(), nodeId_, &nodeClass);
        if (status!= UA_STATUSCODE_GOOD)
            throw std::runtime_error("Failed to verify node existence");

        nodeClass_ = static_cast<NodeClass>(nodeClass);
    }
}

NodeClient::NodeClient(NodeClient const& other){
    client_ = other.client_;
    nodeId_ = other.nodeId_;
    nodeClass_ = other.nodeClass_;
    browseName_ = other.browseName_;
    displayName_ = other.displayName_;
    isForwardReference_ = other.isForwardReference_;
    namespaceIndex_ = other.namespaceIndex_;
}

NodeClient::NodeClient(NodeClient& other){
    client_ = other.client_;
    nodeId_ = other.nodeId_;
    nodeClass_ = other.nodeClass_;
    browseName_ = other.browseName_;
    displayName_ = other.displayName_;
    isForwardReference_ = other.isForwardReference_;
    namespaceIndex_ = other.namespaceIndex_;
}
NodeClient& NodeClient::operator=(NodeClient const& other){
    client_ = other.client_;
    nodeId_ = other.nodeId_;
    nodeClass_ = other.nodeClass_;
    browseName_ = other.browseName_;
    displayName_ = other.displayName_;
    isForwardReference_ = other.isForwardReference_;
    namespaceIndex_ = other.namespaceIndex_;
    return *this;
}
NodeClient& NodeClient::operator=(NodeClient& other){
    client_ = other.client_;
    nodeId_ = other.nodeId_;
    nodeClass_ = other.nodeClass_;
    browseName_ = other.browseName_;
    displayName_ = other.displayName_;
    isForwardReference_ = other.isForwardReference_;
    namespaceIndex_ = other.namespaceIndex_;
    return *this;
}

NodeClient::NodeClient(NodeClient&& other){
    client_ = std::move(other.client_);
    nodeId_ = std::move(other.nodeId_);
    nodeClass_ = std::move(other.nodeClass_);
    browseName_ = std::move(other.browseName_);
    displayName_ = std::move(other.displayName_);
    isForwardReference_ = other.isForwardReference_;
    namespaceIndex_ = other.namespaceIndex_;
}
NodeClient& NodeClient::operator=(NodeClient&& other){
    client_ = std::move(other.client_);
    nodeId_ = std::move(other.nodeId_);
    nodeClass_ = std::move(other.nodeClass_);
    browseName_ = std::move(other.browseName_);
    displayName_ = std::move(other.displayName_);
    isForwardReference_ = other.isForwardReference_;
    namespaceIndex_ = other.namespaceIndex_;
    return *this;
}

std::weak_ptr<Client> NodeClient::getClient() noexcept {
    return client_;
}

std::weak_ptr<Client> NodeClient::getClient() const noexcept {
    return client_;
}

const NodeId& NodeClient::getNodeId() const noexcept {
    return nodeId_;
}

NodeClass NodeClient::getNodeClass() const noexcept {
    return nodeClass_;
}

std::string_view NodeClient::getBrowseName() {
    QualifiedName name;

    const auto status = UA_Client_readBrowseNameAttribute(client_->handle(), nodeId_, name.handle());
    detail::throwOnBadStatus(status);
    return name.getName();
}

LocalizedText NodeClient::getDisplayName() {
    LocalizedText text;
    const auto status = UA_Client_readDisplayNameAttribute(client_->handle(), nodeId_, text.handle());
    detail::throwOnBadStatus(status);
    return text;
}

LocalizedText NodeClient::getDescription() {
    LocalizedText text;
    const auto status = UA_Client_readDescriptionAttribute(client_->handle(), nodeId_, text.handle());
    detail::throwOnBadStatus(status);
    return text;
}

uint32_t NodeClient::getWriteMask() {
    uint32_t writeMask = 0;
    const auto status = UA_Client_readWriteMaskAttribute(client_->handle(), nodeId_, &writeMask);
    detail::throwOnBadStatus(status);
    return writeMask;
}

NodeId NodeClient::getDataType() {
    NodeId nodeId(0, 0);
    const auto status = UA_Client_readDataTypeAttribute(client_->handle(), nodeId_, nodeId.handle());
    detail::throwOnBadStatus(status);
    return nodeId;
}

ValueRank NodeClient::getValueRank() {
    int32_t valueRank = 0;
    const auto status = UA_Client_readValueRankAttribute(client_->handle(), nodeId_, &valueRank);
    detail::throwOnBadStatus(status);
    return static_cast<ValueRank>(valueRank);
}

std::vector<uint32_t> NodeClient::getArrayDimensions() {
    size_t sz;
    UA_UInt32* dims;
    const auto status = UA_Client_readArrayDimensionsAttribute(client_->handle(), nodeId_, &sz, &dims);
    detail::throwOnBadStatus(status);

    return fromNativeArray<uint32_t>(dims, sz);;
}

uint8_t NodeClient::getAccessLevel() {
    uint8_t mask = 0;
    const auto status = UA_Client_readAccessLevelAttribute(client_->handle(), nodeId_, &mask);
    detail::throwOnBadStatus(status);
    return mask;
}

void NodeClient::setDisplayName(std::string_view name, std::string_view locale) {
    const auto status = UA_Client_writeDisplayNameAttribute(
        client_->handle(), nodeId_, LocalizedText(name, locale).handle()
    );
    detail::throwOnBadStatus(status);
}

void NodeClient::setDescription(std::string_view name, std::string_view locale) {
    const auto status = UA_Client_writeDescriptionAttribute(
        client_->handle(), nodeId_, LocalizedText(name, locale).handle()
    );
    detail::throwOnBadStatus(status);
}

void NodeClient::setWriteMask(uint32_t mask) {
    const auto status = UA_Client_writeWriteMaskAttribute(client_->handle(), nodeId_, &mask);
    detail::throwOnBadStatus(status);
}

void NodeClient::setDataType(Type type) {
    const auto status = UA_Client_writeDataTypeAttribute(
        client_->handle(), nodeId_, &detail::getUaDataType(type)->typeId
    );
    detail::throwOnBadStatus(status);
}

void NodeClient::setDataType(const NodeId& typeId) {
    const auto status = UA_Client_writeDataTypeAttribute(client_->handle(), *nodeId_.handle(), typeId.handle());
    detail::throwOnBadStatus(status);
}

void NodeClient::setValueRank(ValueRank valueRank) {
    int rank = static_cast<int32_t>(valueRank);
    const auto status = UA_Client_writeValueRankAttribute(
        client_->handle(), nodeId_, &rank
    );
    detail::throwOnBadStatus(status);
}

void NodeClient::setArrayDimensions(const std::vector<uint32_t>& dimensions) {
    Variant variant;
    variant.setArrayCopy(dimensions);
    auto const arr = variant.getArray<UA_UInt32>();
    const auto status = UA_Client_writeArrayDimensionsAttribute(client_->handle(), nodeId_, variant.getArrayLength(), arr);
    detail::throwOnBadStatus(status);
}

void NodeClient::setAccessLevel(uint8_t mask) {
    UA_Byte m = static_cast<UA_Byte>(mask);
    const auto status = UA_Client_writeAccessLevelAttribute(
        client_->handle(), nodeId_, &m
    );
    detail::throwOnBadStatus(status);
}

// void NodeClient::setModellingRule(ModellingRule rule) {
//     const auto status = UA_Client_addReference(
//         client_->handle(),  // client
//         nodeId_,  // source id
//         UA_NODEID_NUMERIC(0, UA_NS0ID_HASMODELLINGRULE),  // reference id
//         true,  // forward
//         UA_EXPANDEDNODEID_NUMERIC(0, static_cast<UA_UInt32>(rule)),  // target id
//         true  // forward
//     );
//     detail::throwOnBadStatus(status);
// }

// NodeClient NodeClient::addFolder(const NodeId& id, std::string_view browseName, ReferenceType referenceType) {
//     const auto status = UA_Client_addObjectNode(
//         client_->handle(),  // client
//         id,  // new requested id
//         nodeId_,  // parent id
//         detail::getUaNodeId(referenceType),  // reference id
//         QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
//         UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE),  // type definition
//         UA_ObjectAttributes_default,  // object attributes
//         nullptr,  // node context
//         nullptr  // output new node id
//     );
//     detail::throwOnBadStatus(status);
//     return {client_, id};
// }

// NodeClient NodeClient::addObject(
//     const NodeId& id,
//     std::string_view browseName,
//     const NodeId& objectType,
//     ReferenceType referenceType
// ) {
//     const auto status = UA_Client_addObjectNode(
//         client_->handle(),  // client
//         id,  // new requested id
//         nodeId_,  // parent id
//         detail::getUaNodeId(referenceType),  // reference id
//         QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
//         objectType,  // type definition
//         UA_ObjectAttributes_default,  // object attributes
//         nullptr,  // node context
//         nullptr  // output new node id
//     );
//     detail::throwOnBadStatus(status);
//     return {client_, id};
// }

// NodeClient NodeClient::addVariable(
//     const NodeId& id,
//     std::string_view browseName,
//     const NodeId& variableType,
//     ReferenceType referenceType
// ) {
//     const auto status = UA_Client_addVariableNode(
//         client_->handle(),  // client
//         id,  // new requested id
//         nodeId_,  // parent id
//         detail::getUaNodeId(referenceType),  // reference id
//         QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
//         variableType,  // type definition
//         UA_VariableAttributes_default,  // variable attributes
//         nullptr,  // node context
//         nullptr  // output new node id
//     );
//     detail::throwOnBadStatus(status);
//     return {client_, id};
// }

// NodeClient NodeClient::addProperty(const NodeId& id, std::string_view browseName) {
//     const auto status = UA_Client_addVariableNode(
//         client_->handle(),  // client
//         id,  // new requested id
//         nodeId_,  // parent id
//         UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),  // reference id
//         QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
//         UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE),  // type definition
//         UA_VariableAttributes_default,  // variable attributes
//         nullptr,  // node context
//         nullptr  // output new node id
//     );
//     detail::throwOnBadStatus(status);
//     return {client_, id};
// }

// NodeClient NodeClient::addObjectType(
//     const NodeId& id, std::string_view browseName, ReferenceType referenceType
// ) {
//     const auto status = UA_Client_addObjectTypeNode(
//         client_->handle(),  // client
//         id,  // new requested id
//         nodeId_,  // parent id
//         detail::getUaNodeId(referenceType),  // reference id
//         QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
//         UA_ObjectTypeAttributes_default,  // object attributes
//         nullptr,  // node context
//         nullptr  // output new node id
//     );
//     detail::throwOnBadStatus(status);
//     return {client_, id};
// }

// NodeClient NodeClient::addVariableType(
//     const NodeId& id,
//     std::string_view browseName,
//     const NodeId& variableType,
//     ReferenceType referenceType
// ) {
//     const auto status = UA_Client_addVariableTypeNode(
//         client_->handle(),  // client
//         id,  // new requested id
//         nodeId_,  // parent id
//         detail::getUaNodeId(referenceType),  // reference id
//         QualifiedName(id.getNamespaceIndex(), browseName),  // browse name
//         variableType,  // type definition
//         UA_VariableTypeAttributes_default,  // variable attributes
//         nullptr,  // node context
//         nullptr  // output new node id
//     );
//     detail::throwOnBadStatus(status);
//     return {client_, id};
// }

// void NodeClient::addReference(const NodeId& target, ReferenceType referenceType, bool forward) {
//     const auto status = UA_Client_addReference(
//         client_->handle(),  // client
//         nodeId_,  // source id
//         detail::getUaNodeId(referenceType),  // reference id
//         ExpandedNodeId(target, {}, 0),  // target id
//         forward  // forward
//     );
//     detail::throwOnBadStatus(status);
// }

// NodeClient NodeClient::getChild(const std::vector<QualifiedName>& path) {
//     const std::vector<UA_QualifiedName> pathNative(path.begin(), path.end());
//     const TypeWrapper<UA_BrowsePathResult, UA_TYPES_BROWSEPATHRESULT> result(
//         UA_Client_browseSimplifiedBrowsePath(
//             client_->handle(),
//             nodeId_,  // origin
//             pathNative.size(),  // browse path size
//             pathNative.data()  // browse path
//         )
//     );
//     detail::throwOnBadStatus(result->statusCode);
//     assert(result->targets != nullptr && result->targetsSize >= 1);  // NOLINT
//     const auto id = ExpandedNodeId(result->targets[0].targetId);  // NOLINT
//     if (!id.isLocal()) {
//         throw BadStatus(UA_STATUSCODE_BADNOMATCH);
//     }
//     return {client_, id.getNodeId()};
// }

void NodeClient::writeValue(const Variant& var) {
    const auto status = UA_Client_writeValueAttribute(client_->handle(), nodeId_, var.handle());
    detail::throwOnBadStatus(status);
}

void NodeClient::readValue(Variant& var) {
    const auto status = UA_Client_readValueAttribute(client_->handle(), nodeId_, var.handle());
    detail::throwOnBadStatus(status);
}

//std::future<Variant> NodeClient::readValueAsync()
//{
//
//}

void NodeClient::remove(bool deleteReferences) {
    const auto status = UA_Client_deleteNode(client_->handle(), *nodeId_.handle(), deleteReferences);
    detail::throwOnBadStatus(status);
}

void NodeClient::setBrowseName(uint16_t namespaceIndex, const std::string& browseName) {
    browseName_ = browseName;
    namespaceIndex_ = namespaceIndex;
}

void NodeClient::setDisplayName1(const std::string& displayName) {
    displayName_ = displayName;
}

bool NodeClient::isForwardReference() const {
    return isForwardReference_;
}

void NodeClient::setIsForwardReference(bool isForwardReference) {
    isForwardReference_ = isForwardReference;
}

/* ---------------------------------------------------------------------------------------------- */

bool operator==(const NodeClient& left, const NodeClient& right) noexcept {
    return left.getNodeId() == right.getNodeId();
}

bool operator!=(const NodeClient& left, const NodeClient& right) noexcept {
    return !(left == right);
}

}  // namespace opcua