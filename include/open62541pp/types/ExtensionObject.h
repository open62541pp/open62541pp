#pragma once

#include <optional>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeRegistry.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/types/Builtin.h"  // ByteString
#include "open62541pp/types/NodeId.h"

namespace opcua {

/**
 * Extension object encoding.
 * @see UA_ExtensionObjectEncoding
 */
enum class ExtensionObjectEncoding {
    // clang-format off
    EncodedNoBody     = UA_EXTENSIONOBJECT_ENCODED_NOBODY,
    EncodedByteString = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING,
    EncodedXml        = UA_EXTENSIONOBJECT_ENCODED_XML,
    Decoded           = UA_EXTENSIONOBJECT_DECODED,
    DecodedNoDelete   = UA_EXTENSIONOBJECT_DECODED_NODELETE
    // clang-format on
};

/**
 * UA_ExtensionObject wrapper class.
 *
 * ExtensionObjects may contain scalars of any data type. Even those that are unknown to the
 * receiver. If the received data type is unknown, the encoded string and target NodeId is stored
 * instead of the decoded data.
 *
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.1.6
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.2.2.15
 * @ingroup Wrapper
 */
class ExtensionObject : public TypeWrapper<UA_ExtensionObject, UA_TYPES_EXTENSIONOBJECT> {
public:
    using TypeWrapper::TypeWrapper;  // inherit constructors

    /// Create an ExtensionObject from a decoded object (reference).
    /// The data will *not* be deleted when the ExtensionObject is destructed.
    /// @param data Decoded data
    template <typename T>
    [[nodiscard]] static ExtensionObject fromDecoded(T& data) noexcept {
        return fromDecoded(&data, getDataType<T>());
    }

    /// Create an ExtensionObject from a decoded object (reference).
    /// The data will *not* be deleted when the ExtensionObject is destructed.
    /// @param data Decoded data
    /// @param type Data type of the decoded data
    /// @warning Type erased version, use with caution.
    [[nodiscard]] static ExtensionObject fromDecoded(void* data, const UA_DataType& type) noexcept {
        ExtensionObject obj;
        obj->encoding = UA_EXTENSIONOBJECT_DECODED_NODELETE;
        obj->content.decoded.type = &type;  // NOLINT
        obj->content.decoded.data = data;  // NOLINT
        return obj;
    }

    /// Create an ExtensionObject from a decoded object (copy).
    /// Set the "decoded" data to a copy of the given object.
    /// @param data Decoded data
    template <typename T>
    [[nodiscard]] static ExtensionObject fromDecodedCopy(const T& data) {
        return fromDecodedCopy(&data, getDataType<T>());
    }

    /// Create an ExtensionObject from a decoded object (copy).
    /// @param data Decoded data
    /// @param type Data type of the decoded data
    /// @warning Type erased version, use with caution.
    [[nodiscard]] static ExtensionObject fromDecodedCopy(
        const void* data, const UA_DataType& type
    ) {
        // manual implementation instead of UA_ExtensionObject_setValueCopy to support open62541
        // v1.0 https://github.com/open62541/open62541/blob/v1.3.5/src/ua_types.c#L503-L524
        ExtensionObject obj;
        obj->encoding = UA_EXTENSIONOBJECT_DECODED;
        obj->content.decoded.data = detail::allocate<void>(type);  // NOLINT
        obj->content.decoded.type = &type;  // NOLINT
        throwIfBad(UA_copy(data, obj->content.decoded.data, &type));  // NOLINT
        return obj;
    }

    /// Check if the ExtensionObject is empty
    bool isEmpty() const noexcept {
        return (handle()->encoding == UA_EXTENSIONOBJECT_ENCODED_NOBODY);
    }

    /// Check if the ExtensionObject is encoded (usually if the data type is unknown).
    bool isEncoded() const noexcept {
        return (handle()->encoding == UA_EXTENSIONOBJECT_ENCODED_BYTESTRING) ||
               (handle()->encoding == UA_EXTENSIONOBJECT_ENCODED_XML);
    }

    /// Check if the ExtensionObject is decoded.
    bool isDecoded() const noexcept {
        return (handle()->encoding == UA_EXTENSIONOBJECT_DECODED) ||
               (handle()->encoding == UA_EXTENSIONOBJECT_DECODED_NODELETE);
    }

    /// Get the encoding.
    ExtensionObjectEncoding getEncoding() const noexcept {
        return static_cast<ExtensionObjectEncoding>(handle()->encoding);
    }

    /// Get the encoded type id.
    /// Returns `nullptr` if ExtensionObject is not encoded.
    const NodeId* getEncodedTypeId() const noexcept {
        return isEncoded() ? asWrapper<NodeId>(&handle()->content.encoded.typeId)  // NOLINT
                           : nullptr;
    }

    /// Get the encoded body.
    /// Returns `nullptr` if ExtensionObject is not encoded.
    const ByteString* getEncodedBody() const noexcept {
        return isEncoded() ? asWrapper<ByteString>(&handle()->content.encoded.body)  // NOLINT
                           : nullptr;
    }

    /// Get the decoded data type.
    /// Returns `nullptr` if ExtensionObject is not decoded.
    const UA_DataType* getDecodedDataType() const noexcept {
        return isDecoded() ? handle()->content.decoded.type  // NOLINT
                           : nullptr;
    }

    /// Get pointer to the decoded data with given template type.
    /// Returns `nullptr` if the ExtensionObject is either not decoded or the decoded data is not of
    /// type `T`.
    template <typename T>
    T* getDecodedData() noexcept {
        return isDecodedDataType<T>() ? static_cast<T*>(getDecodedData()) : nullptr;
    }

    /// Get const pointer to the decoded data with given template type.
    /// Returns `nullptr` if the ExtensionObject is either not decoded or the decoded data is not of
    /// type `T`.
    template <typename T>
    const T* getDecodedData() const noexcept {
        return isDecodedDataType<T>() ? static_cast<const T*>(getDecodedData()) : nullptr;
    }

    /// Get pointer to the decoded data.
    /// Returns `nullptr` if the ExtensionObject is not decoded.
    /// @warning Type erased version, use with caution.
    void* getDecodedData() noexcept {
        return isDecoded() ? handle()->content.decoded.data  // NOLINT
                           : nullptr;
    }

    /// Get pointer to the decoded data.
    /// Returns `nullptr` if the ExtensionObject is not decoded.
    /// @warning Type erased version, use with caution.
    const void* getDecodedData() const noexcept {
        return isDecoded() ? handle()->content.decoded.data  // NOLINT
                           : nullptr;
    }

private:
    template <typename T>
    bool isDecodedDataType() const noexcept {
        const auto* type = getDecodedDataType();
        return (type != nullptr) && (type->typeId == getDataType<T>().typeId);
    }
};

}  // namespace opcua
