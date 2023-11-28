#pragma once

#include <type_traits>

#include "open62541pp/Common.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/traits.h"
#include "open62541pp/open62541.h"

namespace opcua {

/**
 * Type registry.
 * The type registry is used to derive the corresponding `UA_DataType` object from template types.
 * Custom data types can be registered with template specializations:
 * @code
 * namespace ::opcua {
 * template <>
 * struct TypeRegistry<MyCustomType> {
 *     static const UA_DataType& getDataType() noexcept {
 *         // ...
 *     }
 * };
 * }
 * @endcode
 */
template <typename T, typename Enabled = void>
struct TypeRegistry;

/* -------------------------------------- Traits and helper ------------------------------------- */

namespace detail {

template <typename T, typename = void>
struct IsRegisteredType : std::false_type {};

template <typename T>
struct IsRegisteredType<T, std::void_t<decltype(TypeRegistry<T>{})>> : std::true_type {};

template <typename T>
inline constexpr bool isRegisteredType = IsRegisteredType<T>::value;

template <typename T>
inline const UA_DataType& getDataType() noexcept {
    using ValueType = typename std::remove_cv_t<T>;
    static_assert(
        isRegisteredType<ValueType>,
        "The provided template type is not registered. "
        "Specify the data type manually or add a template specialization for TypeRegistry."
    );
    return TypeRegistry<ValueType>::getDataType();
}

}  // namespace detail

/* ---------------------------------- Template specializations ---------------------------------- */

template <typename T>
struct TypeRegistry<T, std::enable_if_t<detail::isTypeWrapper<T>>> {
    static const UA_DataType& getDataType() noexcept {
        return UA_TYPES[T::getTypeIndex()];
    }
};

// NOLINTNEXTLINE
#define UAPP_TYPEREGISTRY_NATIVE(NativeType, typeIndex)                                            \
    template <>                                                                                    \
    struct TypeRegistry<NativeType> {                                                              \
        static const auto& getDataType() noexcept {                                                \
            return UA_TYPES[typeIndex];                                                            \
        }                                                                                          \
    };

// builtin types
// @cond HIDDEN_SYMBOLS
UAPP_TYPEREGISTRY_NATIVE(UA_Boolean, UA_TYPES_BOOLEAN)
UAPP_TYPEREGISTRY_NATIVE(UA_SByte, UA_TYPES_SBYTE)
UAPP_TYPEREGISTRY_NATIVE(UA_Byte, UA_TYPES_BYTE)
UAPP_TYPEREGISTRY_NATIVE(UA_Int16, UA_TYPES_INT16)
UAPP_TYPEREGISTRY_NATIVE(UA_UInt16, UA_TYPES_UINT16)
UAPP_TYPEREGISTRY_NATIVE(UA_Int32, UA_TYPES_INT32)
UAPP_TYPEREGISTRY_NATIVE(UA_UInt32, UA_TYPES_UINT32)
UAPP_TYPEREGISTRY_NATIVE(UA_Int64, UA_TYPES_INT64)
UAPP_TYPEREGISTRY_NATIVE(UA_UInt64, UA_TYPES_UINT64)
UAPP_TYPEREGISTRY_NATIVE(UA_Float, UA_TYPES_FLOAT)
UAPP_TYPEREGISTRY_NATIVE(UA_Double, UA_TYPES_DOUBLE)
// UAPP_TYPEREGISTRY_NATIVE(UA_String, UA_TYPES_STRING)  // manual implementation below
// UAPP_TYPEREGISTRY_NATIVE(UA_DateTime, UA_TYPES_DATETIME)  // alias for int64_t
UAPP_TYPEREGISTRY_NATIVE(UA_Guid, UA_TYPES_GUID)
// UAPP_TYPEREGISTRY_NATIVE(UA_ByteString, UA_TYPES_BYTESTRING)  // alias for UA_String
// UAPP_TYPEREGISTRY_NATIVE(UA_XmlElement, UA_TYPES_XMLELEMENT)  // alias for UA_String
UAPP_TYPEREGISTRY_NATIVE(UA_NodeId, UA_TYPES_NODEID)
UAPP_TYPEREGISTRY_NATIVE(UA_ExpandedNodeId, UA_TYPES_EXPANDEDNODEID)
// UAPP_TYPEREGISTRY_NATIVE(UA_StatusCode, UA_TYPES_STATUSCODE)  // alias for uint32_t
UAPP_TYPEREGISTRY_NATIVE(UA_QualifiedName, UA_TYPES_QUALIFIEDNAME)
UAPP_TYPEREGISTRY_NATIVE(UA_LocalizedText, UA_TYPES_LOCALIZEDTEXT)
UAPP_TYPEREGISTRY_NATIVE(UA_ExtensionObject, UA_TYPES_EXTENSIONOBJECT)
UAPP_TYPEREGISTRY_NATIVE(UA_DataValue, UA_TYPES_DATAVALUE)
UAPP_TYPEREGISTRY_NATIVE(UA_Variant, UA_TYPES_VARIANT)
UAPP_TYPEREGISTRY_NATIVE(UA_DiagnosticInfo, UA_TYPES_DIAGNOSTICINFO)

template <>
struct TypeRegistry<UA_String> {
    static_assert(std::is_same_v<UA_String, UA_ByteString>);
    static_assert(std::is_same_v<UA_String, UA_XmlElement>);

    template <typename... Ts>
    static const auto& getDataType([[maybe_unused]] Ts... args) noexcept {
        static_assert(
            detail::AlwaysFalse<Ts...>::value,
            "Data type of UA_String is ambiguous (alias for UA_ByteString and UA_XmlElement). "
            "Please specify data type manually."
        );
    }
};

// @endcond

}  // namespace opcua

// include template specializations for native types
#include "open62541pp/TypeRegistryNative.h"
