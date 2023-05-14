#pragma once

#include <optional>

#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/open62541.h"
#include "open62541pp/types/Builtin.h"
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
 * @ingroup TypeWrapper
 */
class ExtensionObject : public TypeWrapper<UA_ExtensionObject, UA_TYPES_EXTENSIONOBJECT> {
public:
    // NOLINTNEXTLINE, false positive?
    using TypeWrapperBase::TypeWrapperBase;  // inherit contructors

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

    /// Get pointer to the encoded data with given template type. Returns `nullptr` if the
    /// ExtensionObject is either encoded or the decoded data not of type `T`.
    template <typename T, TypeIndex typeIndex = detail::guessTypeIndex<T>()>
    T* getDecodedData() noexcept;

    /// Get pointer to the encoded data. Returns `nullptr` if the ExtensionObject is encoded.
    /// @warning Type erased version, use with caution.
    void* getDecodedData() noexcept;

    /// Assign an object to the ExtensionObject (decoded data).
    /// The data will *not* be deleted when the ExtensionObject is destructed.
    /// @param data Decoded data
    template <typename T, TypeIndex typeIndex = detail::guessTypeIndex<T>()>
    void setValue(T& data) noexcept;

    /// Assign an object to the ExtensionObject (decoded data).
    /// The data will *not* be deleted when the ExtensionObject is destructed.
    /// @param data Decoded data
    /// @param type Data type of the decoded data
    /// @warning Type erased version, use with caution.
    void setValue(void* data, const UA_DataType* type) noexcept;

    /// Copy an object to the ExtensionObject (decoded data).
    /// Set the "decoded" data to a copy of the given object.
    /// @param data Decoded data
    template <typename T, TypeIndex typeIndex = detail::guessTypeIndex<T>()>
    void setValueCopy(const T& data);

    /// Copy an object to the ExtensionObject (decoded data).
    /// @param data Decoded data
    /// @param type Data type of the decoded data
    /// @warning Type erased version, use with caution.
    void setValueCopy(const void* data, const UA_DataType* type);
};

/* ------------------------------------------- Helper ------------------------------------------- */

namespace detail {

template <typename T>
constexpr bool isAssignableToExtensionObject() {
    return detail::isBuiltinType<T>() || detail::IsTypeWrapper<T>::value;
}

}  // namespace detail

/* --------------------------------------- Implementation --------------------------------------- */

template <typename T, TypeIndex typeIndex>
T* ExtensionObject::getDecodedData() noexcept {
    detail::assertTypeCombination<T, typeIndex>();
    if (getDecodedDataType() == detail::getUaDataType(typeIndex)) {
        return static_cast<T*>(getDecodedData());
    }
    return nullptr;
}

template <typename T, TypeIndex typeIndex>
void ExtensionObject::setValue(T& data) noexcept {
    detail::assertTypeCombination<T, typeIndex>();
    static_assert(
        detail::isAssignableToExtensionObject<T>(),
        "Template type must be convertible to native type to assign data without copy"
    );
    if constexpr (detail::IsTypeWrapper<T>::value) {
        setValue(data.handle(), detail::getUaDataType<typeIndex>());
    } else {
        setValue(&data, detail::getUaDataType<typeIndex>());
    }
}

template <typename T, TypeIndex typeIndex>
void ExtensionObject::setValueCopy(const T& data) {
    detail::assertTypeCombination<T, typeIndex>();
    if constexpr (detail::IsTypeWrapper<T>::value) {
        setValueCopy(data.handle(), detail::getUaDataType<typeIndex>());
    } else {
        setValueCopy(&data, detail::getUaDataType<typeIndex>());
    }
}

}  // namespace opcua
