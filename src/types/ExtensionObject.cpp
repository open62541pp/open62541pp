#include "open62541pp/types/ExtensionObject.h"

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/NodeId.h"

namespace opcua {

ExtensionObject ExtensionObject::fromDecoded(void* data, const UA_DataType& type) noexcept {
    ExtensionObject obj;
    obj->encoding = UA_EXTENSIONOBJECT_DECODED_NODELETE;
    obj->content.decoded.type = &type;  // NOLINT
    obj->content.decoded.data = data;  // NOLINT
    return obj;
}

ExtensionObject ExtensionObject::fromDecodedCopy(const void* data, const UA_DataType& type) {
    // manual implementation instead of UA_ExtensionObject_setValueCopy to support open62541 v1.0
    // https://github.com/open62541/open62541/blob/v1.3.5/src/ua_types.c#L503-L524
    void* dataCopy = UA_malloc(type.memSize);  // NOLINT
    if (dataCopy == nullptr) {
        throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
    }
    const auto status = UA_copy(data, dataCopy, &type);
    if (detail::isBadStatus(status)) {
        UA_delete(dataCopy, &type);
        throw BadStatus(status);
    }

    ExtensionObject obj;
    obj->encoding = UA_EXTENSIONOBJECT_DECODED;
    obj->content.decoded.data = dataCopy;  // NOLINT
    obj->content.decoded.type = &type;  // NOLINT
    return obj;
}

bool ExtensionObject::isEmpty() const noexcept {
    return (handle()->encoding == UA_EXTENSIONOBJECT_ENCODED_NOBODY);
}

bool ExtensionObject::isEncoded() const noexcept {
    return (handle()->encoding == UA_EXTENSIONOBJECT_ENCODED_BYTESTRING) ||
           (handle()->encoding == UA_EXTENSIONOBJECT_ENCODED_XML);
}

bool ExtensionObject::isDecoded() const noexcept {
    return (handle()->encoding == UA_EXTENSIONOBJECT_DECODED) ||
           (handle()->encoding == UA_EXTENSIONOBJECT_DECODED_NODELETE);
}

ExtensionObjectEncoding ExtensionObject::getEncoding() const noexcept {
    return static_cast<ExtensionObjectEncoding>(handle()->encoding);
}

std::optional<NodeId> ExtensionObject::getEncodedTypeId() const noexcept {
    if (isEmpty() || isEncoded()) {
        return NodeId(handle()->content.encoded.typeId);  // NOLINT
    }
    return {};
}

std::optional<ByteString> ExtensionObject::getEncodedBody() const noexcept {
    if (isEmpty() || isEncoded()) {
        return ByteString(handle()->content.encoded.body);  // NOLINT
    }
    return {};
}

const UA_DataType* ExtensionObject::getDecodedDataType() const noexcept {
    if (isDecoded()) {
        return handle()->content.decoded.type;  // NOLINT
    }
    return nullptr;
}

void* ExtensionObject::getDecodedData() noexcept {
    if (isDecoded()) {
        return handle()->content.decoded.data;  // NOLINT
    }
    return nullptr;
}

const void* ExtensionObject::getDecodedData() const noexcept {
    if (isDecoded()) {
        return handle()->content.decoded.data;  // NOLINT
    }
    return nullptr;
}

}  // namespace opcua
