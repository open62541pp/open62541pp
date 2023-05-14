#include "open62541pp/types/ExtensionObject.h"

#include "open62541pp/ErrorHandling.h"

#include "../open62541_impl.h"

namespace opcua {

bool ExtensionObject::isEncoded() const noexcept {
    return (handle()->encoding < UA_EXTENSIONOBJECT_DECODED);
}

bool ExtensionObject::isDecoded() const noexcept {
    return (handle()->encoding >= UA_EXTENSIONOBJECT_DECODED);
}

ExtensionObjectEncoding ExtensionObject::getEncoding() const noexcept {
    return static_cast<ExtensionObjectEncoding>(handle()->encoding);
}

std::optional<NodeId> ExtensionObject::getEncodedTypeId() const noexcept {
    if (isEncoded()) {
        return NodeId(handle()->content.encoded.typeId);  // NOLINT
    }
    return {};
}

std::optional<ByteString> ExtensionObject::getEncodedBody() const noexcept {
    if (isEncoded()) {
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

void ExtensionObject::setValue(void* data, const UA_DataType* type) noexcept {
    clear();
    handle()->encoding = UA_EXTENSIONOBJECT_DECODED_NODELETE;
    handle()->content.decoded.type = type;  // NOLINT
    handle()->content.decoded.data = data;  // NOLINT
}

void ExtensionObject::setValueCopy(const void* data, const UA_DataType* type) {
    // manual implementation instead of UA_ExtensionObject_setValueCopy to support open62541 v1.0
    // https://github.com/open62541/open62541/blob/v1.3.5/src/ua_types.c#L503-L524
    clear();

    void* dataCopy = UA_malloc(type->memSize);  // NOLINT
    if (dataCopy == nullptr) {
        throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
    }

    const auto status = UA_copy(data, dataCopy, type);
    if (detail::isBadStatus(status)) {
        UA_delete(dataCopy, type);
        throw BadStatus(status);
    }

    handle()->encoding = UA_EXTENSIONOBJECT_DECODED;
    handle()->content.decoded.data = dataCopy;  // NOLINT
    handle()->content.decoded.type = type;  // NOLINT
}

}  // namespace opcua
