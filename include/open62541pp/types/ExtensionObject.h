#pragma once

#include <optional>

#include "open62541pp/TypeRegistry.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"

namespace opcua {

// forward declarations
class ByteString;
class NodeId;

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
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit constructors

    /// Create an ExtensionObject from a decoded object (assign).
    /// The data will *not* be deleted when the ExtensionObject is destructed.
    /// @param data Decoded data
    template <typename T>
    [[nodiscard]] static ExtensionObject fromDecoded(T& data) noexcept {
        return fromDecoded(&data, getDataType<T>());
    }

    /// Create an ExtensionObject from a decoded object (assign).
    /// The data will *not* be deleted when the ExtensionObject is destructed.
    /// @param data Decoded data
    /// @param type Data type of the decoded data
    /// @warning Type erased version, use with caution.
    [[nodiscard]] static ExtensionObject fromDecoded(void* data, const UA_DataType& type) noexcept;

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
    [[nodiscard]] static ExtensionObject fromDecodedCopy(const void* data, const UA_DataType& type);

    /// Check if the ExtensionObject is empty
    bool isEmpty() const noexcept;
    /// Check if the ExtensionObject is encoded (usually if the data type is unknown).
    bool isEncoded() const noexcept;
    /// Check if the ExtensionObject is decoded.
    bool isDecoded() const noexcept;

    /// Get the encoding.
    ExtensionObjectEncoding getEncoding() const noexcept;

    /// Get the encoded type id. Returns `std::nullopt` if ExtensionObject is decoded.
    std::optional<NodeId> getEncodedTypeId() const noexcept;

    /// Get the encoded body. Returns `std::nullopt` if ExtensionObject is decoded.
    std::optional<ByteString> getEncodedBody() const noexcept;

    /// Get the decoded data type. Returns `nullptr` if ExtensionObject is encoded.
    const UA_DataType* getDecodedDataType() const noexcept;

    /// Get pointer to the decoded data with given template type. Returns `nullptr` if the
    /// ExtensionObject is either encoded or the decoded data not of type `T`.
    template <typename T>
    T* getDecodedData() noexcept {
        if (getDecodedDataType() == &getDataType<T>()) {
            return static_cast<T*>(getDecodedData());
        }
        return nullptr;
    }

    /// @copydoc getDecodedData
    template <typename T>
    const T* getDecodedData() const noexcept {
        if (getDecodedDataType() == &getDataType<T>()) {
            return static_cast<const T*>(getDecodedData());
        }
        return nullptr;
    }

    /// Get pointer to the decoded data. Returns `nullptr` if the ExtensionObject is encoded.
    /// @warning Type erased version, use with caution.
    void* getDecodedData() noexcept;

    /// @copydoc getDecodedData
    const void* getDecodedData() const noexcept;
};

}  // namespace opcua
