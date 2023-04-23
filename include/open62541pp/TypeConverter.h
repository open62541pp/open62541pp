#pragma once

#include <array>
#include <chrono>
#include <iterator>  // distance
#include <string>
#include <type_traits>
#include <vector>

#include "open62541pp/Common.h"
#include "open62541pp/ErrorHandling.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/helper.h"
#include "open62541pp/detail/traits.h"
#include "open62541pp/open62541.h"
#include "open62541pp/types/DateTime.h"

namespace opcua {

template <TypeIndex... typeIndexes>
struct TypeIndexList {
    using TypeIndexes = std::integer_sequence<TypeIndex, typeIndexes...>;

    static constexpr size_t size() {
        return sizeof...(typeIndexes);
    }

    static constexpr bool contains(TypeIndex typeIndex) {
        return ((typeIndex == typeIndexes) || ...);
    }

    static constexpr bool contains(Type type) {
        return contains(static_cast<TypeIndex>(type));
    }

    static constexpr auto toArray() {
        return std::array<TypeIndex, sizeof...(typeIndexes)>{typeIndexes...};
    }
};

template <typename T, typename Enable = void>
struct TypeConverter {
    static_assert(detail::AlwaysFalse<T>::value, "Missing specialization of TypeConverter");

    using ValueType = T;
    using NativeType = std::nullptr_t;
    using ValidTypes = TypeIndexList<>;

    static void fromNative(const NativeType& src, ValueType& dst);
    static void toNative(const ValueType& src, NativeType& dst);
};

/* ------------------------------------------- Helper ------------------------------------------- */

namespace detail {

template <typename T, typename TypeIndexOrType>
constexpr bool isValidTypeCombination(TypeIndexOrType typeOrTypeIndex) {
    return TypeConverter<T>::ValidTypes::contains(typeOrTypeIndex);
}

template <typename T, auto typeOrTypeIndex>
constexpr void assertTypeCombination() {
    static_assert(
        isValidTypeCombination<T>(typeOrTypeIndex), "Invalid template type / type index combination"
    );
}

template <typename T>
constexpr TypeIndex guessTypeIndex() {
    using ValueType = typename std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(
        TypeConverter<ValueType>::ValidTypes::size() == 1,
        "Ambiguous template type, please specify type index (UA_TYPES_*) manually"
    );
    return TypeConverter<ValueType>::ValidTypes::toArray().at(0);
}

template <typename T>
constexpr Type guessType() {
    using ValueType = typename std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(
        TypeConverter<ValueType>::ValidTypes::size() == 1,
        "Ambiguous template type, please specify type enum (opcua::Type) manually"
    );
    constexpr auto typeIndexGuess = TypeConverter<ValueType>::ValidTypes::toArray().at(0);
    static_assert(typeIndexGuess < builtinTypesCount, "T doesn't seem to be a builtin type");
    return static_cast<Type>(typeIndexGuess);
}

template <typename It>
constexpr TypeIndex guessTypeIndexFromIterator() {
    using ValueType = typename std::iterator_traits<It>::value_type;
    return guessTypeIndex<ValueType>();
}

template <typename It>
constexpr Type guessTypeFromIterator() {
    using ValueType = typename std::iterator_traits<It>::value_type;
    return guessType<ValueType>();
}

/* ------------------------------------- Converter functions ------------------------------------ */

/// Convert and copy from native type.
template <typename T, typename NativeType = typename TypeConverter<T>::NativeType>
[[nodiscard]] T fromNative(NativeType* value) {
    T result{};
    TypeConverter<T>::fromNative(*value, result);
    return result;
}

/// Convert and copy from native type.
/// @warning Type erased version, use with caution.
template <typename T, typename NativeType = typename TypeConverter<T>::NativeType>
[[nodiscard]] T fromNative(void* value, [[maybe_unused]] Type type) {
    assert(isValidTypeCombination<T>(type));  // NOLINT
    return fromNative<T>(static_cast<NativeType*>(value));
}

/// Create and convert vector from native array.
template <typename T, typename NativeType = typename TypeConverter<T>::NativeType>
[[nodiscard]] std::vector<T> fromNativeArray(NativeType* array, size_t size) {
    if constexpr (isBuiltinType<T>() && std::is_fundamental_v<T>) {
        return std::vector<T>(array, array + size);  // NOLINT
    } else {
        std::vector<T> result(size);
        for (size_t i = 0; i < size; ++i) {
            TypeConverter<T>::fromNative(array[i], result[i]);  // NOLINT
        }
        return result;
    }
}

/// Create and convert vector from native array.
/// @warning Type erased version, use with caution.
template <typename T, typename NativeType = typename TypeConverter<T>::NativeType>
[[nodiscard]] std::vector<T> fromNativeArray(void* array, size_t size, [[maybe_unused]] Type type) {
    assert(isValidTypeCombination<T>(type));  // NOLINT
    return fromNativeArray<T>(static_cast<NativeType*>(array), size);
}

/// Allocate native type.
template <typename TNative, TypeIndex typeIndex = guessTypeIndex<TNative>()>
[[nodiscard]] TNative* allocNative() {
    assertTypeCombination<TNative, typeIndex>();
    auto* result = static_cast<TNative*>(UA_new(getUaDataType<typeIndex>()));
    if (result == nullptr) {
        throw std::bad_alloc();
    }
    return result;
}

/// Allocate and copy to native type.
template <typename T, TypeIndex typeIndex = guessTypeIndex<T>()>
[[nodiscard]] auto* toNativeAlloc(const T& value) {
    assertTypeCombination<T, typeIndex>();
    using NativeType = typename TypeConverter<T>::NativeType;
    auto* result = allocNative<NativeType, typeIndex>();
    TypeConverter<T>::toNative(value, *result);
    return result;
}

/// Allocate native array
template <typename TNative, TypeIndex typeIndex = guessTypeIndex<TNative>()>
[[nodiscard]] auto* allocNativeArray(size_t size) {
    assertTypeCombination<TNative, typeIndex>();
    auto* result = static_cast<TNative*>(UA_Array_new(size, getUaDataType<typeIndex>()));
    if (result == nullptr) {
        throw std::bad_alloc();
    }
    return result;
}

/// Allocate and copy iterator range to native array.
template <typename InputIt, TypeIndex typeIndex = guessTypeIndexFromIterator<InputIt>()>
[[nodiscard]] auto* toNativeArrayAlloc(InputIt first, InputIt last) {
    using ValueType = typename std::iterator_traits<InputIt>::value_type;
    using NativeType = typename TypeConverter<ValueType>::NativeType;
    assertTypeCombination<ValueType, typeIndex>();
    const size_t size = std::distance(first, last);
    auto* result = allocNativeArray<NativeType, typeIndex>(size);
    for (size_t i = 0; i < size; ++i) {
        TypeConverter<ValueType>::toNative(*first++, result[i]);  // NOLINT
    }
    return result;
}

/// Allocate and copy to native array.
template <typename T, TypeIndex typeIndex = guessTypeIndex<T>()>
[[nodiscard]] auto* toNativeArrayAlloc(const T* array, size_t size) {
    return toNativeArrayAlloc<const T*, typeIndex>(array, array + size);  // NOLINT
}

}  // namespace detail

/* ---------------------------- Implementation for native data types ---------------------------- */

namespace detail {

template <typename T, TypeIndex... typeIndexes>
struct TypeConverterNative {
    using ValueType = T;
    using NativeType = T;
    using ValidTypes = TypeIndexList<typeIndexes...>;

    static_assert(ValidTypes::size() >= 1);

    static void fromNative(const T& src, T& dst) {
        if constexpr (std::is_fundamental_v<T>) {
            dst = src;
        } else {
            // just take first type -> underlying memory layout of all types should be the same
            constexpr auto typeIndexGuess = ValidTypes::toArray().at(0);
            // clear first
            UA_clear(&dst, getUaDataType<typeIndexGuess>());
            // deep copy
            const auto status = UA_copy(&src, &dst, getUaDataType<typeIndexGuess>());
            throwOnBadStatus(status);
        }
    }

    static void toNative(const T& src, T& dst) {
        fromNative(src, dst);
    }
};

}  // namespace detail

// NOLINTNEXTLINE
#define UAPP_TYPECONVERTER_NATIVE(NativeType, ...)                                                 \
    template <>                                                                                    \
    struct TypeConverter<NativeType> : detail::TypeConverterNative<NativeType, __VA_ARGS__> {};

// clang-format off

// builtin types
UAPP_TYPECONVERTER_NATIVE(UA_Boolean, UA_TYPES_BOOLEAN)
UAPP_TYPECONVERTER_NATIVE(UA_SByte, UA_TYPES_SBYTE)
UAPP_TYPECONVERTER_NATIVE(UA_Byte, UA_TYPES_BYTE)
UAPP_TYPECONVERTER_NATIVE(UA_Int16, UA_TYPES_INT16)
UAPP_TYPECONVERTER_NATIVE(UA_UInt16, UA_TYPES_UINT16)
UAPP_TYPECONVERTER_NATIVE(UA_Int32, UA_TYPES_INT32)
UAPP_TYPECONVERTER_NATIVE(UA_UInt32, UA_TYPES_UINT32)
UAPP_TYPECONVERTER_NATIVE(UA_Int64, UA_TYPES_INT64)
UAPP_TYPECONVERTER_NATIVE(UA_UInt64, UA_TYPES_UINT64)
UAPP_TYPECONVERTER_NATIVE(UA_Float, UA_TYPES_FLOAT)
UAPP_TYPECONVERTER_NATIVE(UA_Double, UA_TYPES_DOUBLE)
static_assert(std::is_same_v<UA_String, UA_ByteString>);
static_assert(std::is_same_v<UA_String, UA_XmlElement>);
UAPP_TYPECONVERTER_NATIVE(UA_String, UA_TYPES_STRING, UA_TYPES_BYTESTRING, UA_TYPES_XMLELEMENT)
UAPP_TYPECONVERTER_NATIVE(UA_Guid, UA_TYPES_GUID)
UAPP_TYPECONVERTER_NATIVE(UA_NodeId, UA_TYPES_NODEID)
UAPP_TYPECONVERTER_NATIVE(UA_ExpandedNodeId, UA_TYPES_EXPANDEDNODEID)
UAPP_TYPECONVERTER_NATIVE(UA_QualifiedName, UA_TYPES_QUALIFIEDNAME)
UAPP_TYPECONVERTER_NATIVE(UA_LocalizedText, UA_TYPES_LOCALIZEDTEXT)
UAPP_TYPECONVERTER_NATIVE(UA_ExtensionObject, UA_TYPES_EXTENSIONOBJECT)
// composed types (minimal namespace zero)
UAPP_TYPECONVERTER_NATIVE(UA_DataValue, UA_TYPES_DATAVALUE)
UAPP_TYPECONVERTER_NATIVE(UA_Variant, UA_TYPES_VARIANT)
UAPP_TYPECONVERTER_NATIVE(UA_DiagnosticInfo, UA_TYPES_DIAGNOSTICINFO)
UAPP_TYPECONVERTER_NATIVE(UA_NodeClass, UA_TYPES_NODECLASS)
UAPP_TYPECONVERTER_NATIVE(UA_ApplicationDescription, UA_TYPES_APPLICATIONDESCRIPTION)
UAPP_TYPECONVERTER_NATIVE(UA_ApplicationType, UA_TYPES_APPLICATIONTYPE)
UAPP_TYPECONVERTER_NATIVE(UA_ChannelSecurityToken, UA_TYPES_CHANNELSECURITYTOKEN)
UAPP_TYPECONVERTER_NATIVE(UA_OpenSecureChannelRequest, UA_TYPES_OPENSECURECHANNELREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_OpenSecureChannelResponse, UA_TYPES_OPENSECURECHANNELRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_CloseSecureChannelRequest, UA_TYPES_CLOSESECURECHANNELREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_CloseSecureChannelResponse, UA_TYPES_CLOSESECURECHANNELRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_RequestHeader, UA_TYPES_REQUESTHEADER)
UAPP_TYPECONVERTER_NATIVE(UA_ResponseHeader, UA_TYPES_RESPONSEHEADER)
UAPP_TYPECONVERTER_NATIVE(UA_SecurityTokenRequestType, UA_TYPES_SECURITYTOKENREQUESTTYPE)
UAPP_TYPECONVERTER_NATIVE(UA_MessageSecurityMode, UA_TYPES_MESSAGESECURITYMODE)
UAPP_TYPECONVERTER_NATIVE(UA_CloseSessionResponse, UA_TYPES_CLOSESESSIONRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_CloseSessionRequest, UA_TYPES_CLOSESESSIONREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_ActivateSessionRequest, UA_TYPES_ACTIVATESESSIONREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_ActivateSessionResponse, UA_TYPES_ACTIVATESESSIONRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_SignatureData, UA_TYPES_SIGNATUREDATA)
UAPP_TYPECONVERTER_NATIVE(UA_SignedSoftwareCertificate, UA_TYPES_SIGNEDSOFTWARECERTIFICATE)
UAPP_TYPECONVERTER_NATIVE(UA_ServiceFault, UA_TYPES_SERVICEFAULT)
UAPP_TYPECONVERTER_NATIVE(UA_UserIdentityToken, UA_TYPES_USERIDENTITYTOKEN)
UAPP_TYPECONVERTER_NATIVE(UA_UserNameIdentityToken, UA_TYPES_USERNAMEIDENTITYTOKEN)
UAPP_TYPECONVERTER_NATIVE(UA_AnonymousIdentityToken, UA_TYPES_ANONYMOUSIDENTITYTOKEN)
UAPP_TYPECONVERTER_NATIVE(UA_X509IdentityToken, UA_TYPES_X509IDENTITYTOKEN)
UAPP_TYPECONVERTER_NATIVE(UA_IssuedIdentityToken, UA_TYPES_ISSUEDIDENTITYTOKEN)
UAPP_TYPECONVERTER_NATIVE(UA_CreateSessionResponse, UA_TYPES_CREATESESSIONRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_CreateSessionRequest, UA_TYPES_CREATESESSIONREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_EndpointDescription, UA_TYPES_ENDPOINTDESCRIPTION)
UAPP_TYPECONVERTER_NATIVE(UA_UserTokenPolicy, UA_TYPES_USERTOKENPOLICY)
UAPP_TYPECONVERTER_NATIVE(UA_UserTokenType, UA_TYPES_USERTOKENTYPE)
UAPP_TYPECONVERTER_NATIVE(UA_GetEndpointsRequest, UA_TYPES_GETENDPOINTSREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_GetEndpointsResponse, UA_TYPES_GETENDPOINTSRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_FindServersRequest, UA_TYPES_FINDSERVERSREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_FindServersResponse, UA_TYPES_FINDSERVERSRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_TimestampsToReturn, UA_TYPES_TIMESTAMPSTORETURN)
UAPP_TYPECONVERTER_NATIVE(UA_ReadRequest, UA_TYPES_READREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_ReadResponse, UA_TYPES_READRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_ReadValueId, UA_TYPES_READVALUEID)
UAPP_TYPECONVERTER_NATIVE(UA_WriteRequest, UA_TYPES_WRITEREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_WriteResponse, UA_TYPES_WRITERESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_WriteValue, UA_TYPES_WRITEVALUE)
UAPP_TYPECONVERTER_NATIVE(UA_TranslateBrowsePathsToNodeIdsRequest, UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_TranslateBrowsePathsToNodeIdsResponse, UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_BrowseResultMask, UA_TYPES_BROWSERESULTMASK)
UAPP_TYPECONVERTER_NATIVE(UA_BrowsePath, UA_TYPES_BROWSEPATH)
UAPP_TYPECONVERTER_NATIVE(UA_BrowsePathResult, UA_TYPES_BROWSEPATHRESULT)
UAPP_TYPECONVERTER_NATIVE(UA_BrowsePathTarget, UA_TYPES_BROWSEPATHTARGET)
UAPP_TYPECONVERTER_NATIVE(UA_RelativePath, UA_TYPES_RELATIVEPATH)
UAPP_TYPECONVERTER_NATIVE(UA_RelativePathElement, UA_TYPES_RELATIVEPATHELEMENT)
UAPP_TYPECONVERTER_NATIVE(UA_BrowseResponse, UA_TYPES_BROWSERESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_BrowseResult, UA_TYPES_BROWSERESULT)
UAPP_TYPECONVERTER_NATIVE(UA_ReferenceDescription, UA_TYPES_REFERENCEDESCRIPTION)
UAPP_TYPECONVERTER_NATIVE(UA_ViewDescription, UA_TYPES_VIEWDESCRIPTION)
UAPP_TYPECONVERTER_NATIVE(UA_BrowseRequest, UA_TYPES_BROWSEREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_BrowseNextRequest, UA_TYPES_BROWSENEXTREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_BrowseNextResponse, UA_TYPES_BROWSENEXTRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_BrowseDescription, UA_TYPES_BROWSEDESCRIPTION)
UAPP_TYPECONVERTER_NATIVE(UA_BrowseDirection, UA_TYPES_BROWSEDIRECTION)
UAPP_TYPECONVERTER_NATIVE(UA_AddNodesRequest, UA_TYPES_ADDNODESREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_AddNodesResponse, UA_TYPES_ADDNODESRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_AddNodesItem, UA_TYPES_ADDNODESITEM)
UAPP_TYPECONVERTER_NATIVE(UA_AddNodesResult, UA_TYPES_ADDNODESRESULT)
UAPP_TYPECONVERTER_NATIVE(UA_AddReferencesRequest, UA_TYPES_ADDREFERENCESREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_AddReferencesResponse, UA_TYPES_ADDREFERENCESRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_AddReferencesItem, UA_TYPES_ADDREFERENCESITEM)
UAPP_TYPECONVERTER_NATIVE(UA_NodeAttributes, UA_TYPES_NODEATTRIBUTES)
UAPP_TYPECONVERTER_NATIVE(UA_VariableAttributes, UA_TYPES_VARIABLEATTRIBUTES)
UAPP_TYPECONVERTER_NATIVE(UA_ObjectAttributes, UA_TYPES_OBJECTATTRIBUTES)
UAPP_TYPECONVERTER_NATIVE(UA_ReferenceTypeAttributes, UA_TYPES_REFERENCETYPEATTRIBUTES)
UAPP_TYPECONVERTER_NATIVE(UA_ViewAttributes, UA_TYPES_VIEWATTRIBUTES)
UAPP_TYPECONVERTER_NATIVE(UA_MethodAttributes, UA_TYPES_METHODATTRIBUTES)
UAPP_TYPECONVERTER_NATIVE(UA_ObjectTypeAttributes, UA_TYPES_OBJECTTYPEATTRIBUTES)
UAPP_TYPECONVERTER_NATIVE(UA_VariableTypeAttributes, UA_TYPES_VARIABLETYPEATTRIBUTES)
UAPP_TYPECONVERTER_NATIVE(UA_DataTypeAttributes, UA_TYPES_DATATYPEATTRIBUTES)
UAPP_TYPECONVERTER_NATIVE(UA_NodeAttributesMask, UA_TYPES_NODEATTRIBUTESMASK)
UAPP_TYPECONVERTER_NATIVE(UA_DeleteNodesItem, UA_TYPES_DELETENODESITEM)
UAPP_TYPECONVERTER_NATIVE(UA_DeleteNodesRequest, UA_TYPES_DELETENODESREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_DeleteNodesResponse, UA_TYPES_DELETENODESRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_DeleteReferencesItem, UA_TYPES_DELETEREFERENCESITEM)
UAPP_TYPECONVERTER_NATIVE(UA_DeleteReferencesRequest, UA_TYPES_DELETEREFERENCESREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_DeleteReferencesResponse, UA_TYPES_DELETEREFERENCESRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_RegisterNodesRequest, UA_TYPES_REGISTERNODESREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_RegisterNodesResponse, UA_TYPES_REGISTERNODESRESPONSE)
UAPP_TYPECONVERTER_NATIVE(UA_UnregisterNodesRequest, UA_TYPES_UNREGISTERNODESREQUEST)
UAPP_TYPECONVERTER_NATIVE(UA_UnregisterNodesResponse, UA_TYPES_UNREGISTERNODESRESPONSE)
// UAPP_TYPECONVERTER_NATIVE(UA_Duration, UA_TYPES_DURATION)
// UAPP_TYPECONVERTER_NATIVE(UA_UtcTime, UA_TYPES_UTCTIME)
// UAPP_TYPECONVERTER_NATIVE(UA_LocaleId, UA_TYPES_LOCALEID)
UAPP_TYPECONVERTER_NATIVE(UA_EnumValueType, UA_TYPES_ENUMVALUETYPE)
UAPP_TYPECONVERTER_NATIVE(UA_BuildInfo, UA_TYPES_BUILDINFO)
UAPP_TYPECONVERTER_NATIVE(UA_ServerStatusDataType, UA_TYPES_SERVERSTATUSDATATYPE)
UAPP_TYPECONVERTER_NATIVE(UA_ServerState, UA_TYPES_SERVERSTATE)
UAPP_TYPECONVERTER_NATIVE(UA_ServerDiagnosticsSummaryDataType, UA_TYPES_SERVERDIAGNOSTICSSUMMARYDATATYPE)
UAPP_TYPECONVERTER_NATIVE(UA_RedundancySupport, UA_TYPES_REDUNDANCYSUPPORT)
UAPP_TYPECONVERTER_NATIVE(UA_KeyValuePair, UA_TYPES_KEYVALUEPAIR)

// clang-format on

/* ------------------------------- Implementation for TypeWrapper ------------------------------- */

template <typename WrapperType>
struct TypeConverter<WrapperType, std::enable_if_t<detail::IsTypeWrapper<WrapperType>::value>> {
    using NativeConverter = TypeConverter<typename WrapperType::NativeType>;

    using ValueType = WrapperType;
    using NativeType = typename WrapperType::NativeType;
    using ValidTypes = TypeIndexList<WrapperType::getTypeIndex()>;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = WrapperType(src);
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        NativeConverter::toNative(*src.handle(), dst);
    }
};

/* ---------------------------- Implementations for std library types --------------------------- */

template <>
struct TypeConverter<std::string> {
    using ValueType = std::string;
    using NativeType = UA_String;
    using ValidTypes = TypeIndexList<UA_TYPES_STRING, UA_TYPES_BYTESTRING, UA_TYPES_XMLELEMENT>;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = detail::toString(src);
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        UA_clear(&dst, detail::getUaDataType<UA_TYPES_STRING>());
        dst = detail::allocUaString(src);
    }
};

template <typename Clock, typename Duration>
struct TypeConverter<std::chrono::time_point<Clock, Duration>> {
    using ValueType = std::chrono::time_point<Clock, Duration>;
    using NativeType = UA_DateTime;
    using ValidTypes = TypeIndexList<UA_TYPES_DATETIME>;

    static void fromNative(const NativeType& src, ValueType& dst) {
        dst = DateTime(src).toTimePoint<Clock, Duration>();
    }

    static void toNative(const ValueType& src, NativeType& dst) {
        dst = DateTime::fromTimePoint(src).get();
    }
};

}  // namespace opcua
