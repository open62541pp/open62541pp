#pragma once

#include <cstdint>
#include <initializer_list>
#include <string_view>
#include <type_traits>
#include <utility>  // forward, move
#include <variant>

#include "open62541pp/Bitmask.h"
#include "open62541pp/Common.h"  // AttributeId, TimestampsToReturn, ...
#include "open62541pp/Config.h"
#include "open62541pp/NodeIds.h"  // ReferenceTypeId
#include "open62541pp/Span.h"
#include "open62541pp/TypeRegistry.h"  // getDataType
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/traits.h"  // IsOneOf
#include "open62541pp/detail/types_conversion.h"  // toNative, toNativeArray
#include "open62541pp/detail/types_handling.h"  // deallocateArray, copyArray
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/ExtensionObject.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

#ifndef UA_DEFAULT_ATTRIBUTES_DEFINED
#define UA_DEFAULT_ATTRIBUTES_DEFINED
extern "C" {
UA_EXPORT extern const UA_VariableAttributes UA_VariableAttributes_default;
UA_EXPORT extern const UA_VariableTypeAttributes UA_VariableTypeAttributes_default;
UA_EXPORT extern const UA_MethodAttributes UA_MethodAttributes_default;
UA_EXPORT extern const UA_ObjectAttributes UA_ObjectAttributes_default;
UA_EXPORT extern const UA_ObjectTypeAttributes UA_ObjectTypeAttributes_default;
UA_EXPORT extern const UA_ReferenceTypeAttributes UA_ReferenceTypeAttributes_default;
UA_EXPORT extern const UA_DataTypeAttributes UA_DataTypeAttributes_default;
UA_EXPORT extern const UA_ViewAttributes UA_ViewAttributes_default;
}
#endif

// NOLINTNEXTLINE
#define UAPP_GETTER(Type, getterName, member)                                                      \
    Type getterName() const noexcept {                                                             \
        return handle()->member;                                                                   \
    }

// NOLINTNEXTLINE
#define UAPP_GETTER_CAST(Type, getterName, member)                                                 \
    Type getterName() const noexcept {                                                             \
        return static_cast<Type>(handle()->member);                                                \
    }

// NOLINTNEXTLINE
#define UAPP_GETTER_WRAPPER_CONST(Type, getterName, member)                                        \
    const Type& getterName() const noexcept {                                                      \
        return asWrapper<Type>(handle()->member);                                                  \
    }
// NOLINTNEXTLINE
#define UAPP_GETTER_WRAPPER_NONCONST(Type, getterName, member)                                     \
    Type& getterName() noexcept {                                                                  \
        return asWrapper<Type>(handle()->member);                                                  \
    }
// NOLINTNEXTLINE
#define UAPP_GETTER_WRAPPER(Type, getterName, member)                                              \
    UAPP_GETTER_WRAPPER_CONST(Type, getterName, member)                                            \
    UAPP_GETTER_WRAPPER_NONCONST(Type, getterName, member)

// NOLINTNEXTLINE
#define UAPP_GETTER_SPAN(Type, getterName, memberArray, memberSize)                                \
    Span<const Type> getterName() const noexcept {                                                 \
        return {handle()->memberArray, handle()->memberSize};                                      \
    }                                                                                              \
    Span<Type> getterName() noexcept {                                                             \
        return {handle()->memberArray, handle()->memberSize};                                      \
    }
// NOLINTNEXTLINE
#define UAPP_GETTER_SPAN_WRAPPER(Type, getterName, memberArray, memberSize)                        \
    Span<const Type> getterName() const noexcept {                                                 \
        return {asWrapper<Type>(handle()->memberArray), handle()->memberSize};                     \
    }                                                                                              \
    Span<Type> getterName() noexcept {                                                             \
        return {asWrapper<Type>(handle()->memberArray), handle()->memberSize};                     \
    }

namespace opcua {

/**
 * @addtogroup Wrapper
 * @{
 */

/**
 * UA_EnumValueType wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.39
 */
class EnumValueType : public TypeWrapper<UA_EnumValueType, UA_TYPES_ENUMVALUETYPE> {
public:
    using TypeWrapper::TypeWrapper;

    EnumValueType(int64_t value, LocalizedText displayName, LocalizedText description) {
        handle()->value = value;
        handle()->displayName = detail::toNative(std::move(displayName));
        handle()->description = detail::toNative(std::move(description));
    }

    UAPP_GETTER(int64_t, getValue, value)
    UAPP_GETTER_WRAPPER(LocalizedText, getDisplayName, displayName)
    UAPP_GETTER_WRAPPER(LocalizedText, getDescription, description)
};

/**
 * UA_ApplicationDescription wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.2
 */
class ApplicationDescription
    : public TypeWrapper<UA_ApplicationDescription, UA_TYPES_APPLICATIONDESCRIPTION> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(String, getApplicationUri, applicationUri)
    UAPP_GETTER_WRAPPER(String, getProductUri, productUri)
    UAPP_GETTER_WRAPPER(LocalizedText, getApplicationName, applicationName)
    UAPP_GETTER(UA_ApplicationType, getApplicationType, applicationType)
    UAPP_GETTER_WRAPPER(String, getGatewayServerUri, gatewayServerUri)
    UAPP_GETTER_WRAPPER(String, getDiscoveryProfileUri, discoveryProfileUri)
    UAPP_GETTER_SPAN_WRAPPER(String, getDiscoveryUrls, discoveryUrls, discoveryUrlsSize)
};

/**
 * UA_RequestHeader wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.33
 */
class RequestHeader : public TypeWrapper<UA_RequestHeader, UA_TYPES_REQUESTHEADER> {
public:
    using TypeWrapper::TypeWrapper;

    RequestHeader(
        NodeId authenticationToken,
        DateTime timestamp,
        uint32_t requestHandle,
        uint32_t returnDiagnostics,
        std::string_view auditEntryId,
        uint32_t timeoutHint,
        ExtensionObject additionalHeader
    ) {
        handle()->authenticationToken = detail::toNative(std::move(authenticationToken));
        handle()->timestamp = timestamp;
        handle()->requestHandle = requestHandle;
        handle()->returnDiagnostics = returnDiagnostics;
        handle()->auditEntryId = detail::toNative(auditEntryId);
        handle()->timeoutHint = timeoutHint;
        handle()->additionalHeader = detail::toNative(std::move(additionalHeader));
    }

    UAPP_GETTER_WRAPPER(NodeId, getAuthenticationToken, authenticationToken)
    UAPP_GETTER_WRAPPER(DateTime, getTimestamp, timestamp)
    UAPP_GETTER(uint32_t, getRequestHandle, requestHandle)
    UAPP_GETTER(uint32_t, getReturnDiagnostics, returnDiagnostics)
    UAPP_GETTER_WRAPPER(String, getAuditEntryId, auditEntryId)
    UAPP_GETTER(uint32_t, getTimeoutHint, timeoutHint)
    UAPP_GETTER_WRAPPER(ExtensionObject, getAdditionalHeader, additionalHeader)
};

/**
 * UA_ResponseHeader wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.34
 */
class ResponseHeader : public TypeWrapper<UA_ResponseHeader, UA_TYPES_RESPONSEHEADER> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(DateTime, getTimestamp, timestamp)
    UAPP_GETTER(uint32_t, getRequestHandle, requestHandle)
    UAPP_GETTER(StatusCode, getServiceResult, serviceResult)
    UAPP_GETTER_WRAPPER(DiagnosticInfo, getServiceDiagnostics, serviceDiagnostics)
    UAPP_GETTER_SPAN_WRAPPER(String, getStringTable, stringTable, stringTableSize)
    UAPP_GETTER_WRAPPER(ExtensionObject, getAdditionalHeader, additionalHeader)
};

/**
 * User identity token type.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.43
 */
enum class UserTokenType : int32_t {
    // clang-format off
    Anonymous   = 0,  ///< No token is required
    Username    = 1,  ///< A username/password token
    Certificate = 2,  ///< An X.509 v3 certificate token
    IssuedToken = 3,  ///< Any token issued by an authorization service
    // clang-format on
};

/**
 * UA_UserTokenPolicy wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.42
 */
class UserTokenPolicy : public TypeWrapper<UA_UserTokenPolicy, UA_TYPES_USERTOKENPOLICY> {
public:
    using TypeWrapper::TypeWrapper;

    UserTokenPolicy(
        std::string_view policyId,
        UserTokenType tokenType,
        std::string_view issuedTokenType,
        std::string_view issuerEndpointUrl,
        std::string_view securityPolicyUri
    ) {
        handle()->policyId = detail::toNative(policyId);
        handle()->tokenType = static_cast<UA_UserTokenType>(tokenType);
        handle()->issuedTokenType = detail::toNative(issuedTokenType);
        handle()->issuerEndpointUrl = detail::toNative(issuerEndpointUrl);
        handle()->securityPolicyUri = detail::toNative(securityPolicyUri);
    }

    UAPP_GETTER_WRAPPER(String, getPolicyId, policyId)
    UAPP_GETTER_CAST(UserTokenType, getTokenType, tokenType)
    UAPP_GETTER_WRAPPER(String, getIssuedTokenType, issuedTokenType)
    UAPP_GETTER_WRAPPER(String, getIssuerEndpointUrl, issuerEndpointUrl)
    UAPP_GETTER_WRAPPER(String, getSecurityPolicyUri, securityPolicyUri)
};

/**
 * UA_EndpointDescription wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.14
 */
class EndpointDescription
    : public TypeWrapper<UA_EndpointDescription, UA_TYPES_ENDPOINTDESCRIPTION> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(String, getEndpointUrl, endpointUrl)
    UAPP_GETTER_WRAPPER(ApplicationDescription, getServer, server)
    UAPP_GETTER_WRAPPER(ByteString, getServerCertificate, serverCertificate)
    UAPP_GETTER(UA_MessageSecurityMode, getSecurityMode, securityMode)
    UAPP_GETTER_WRAPPER(String, getSecurityPolicyUri, securityPolicyUri)
    UAPP_GETTER_SPAN_WRAPPER(
        UserTokenPolicy, getUserIdentityTokens, userIdentityTokens, userIdentityTokensSize
    )
    UAPP_GETTER_WRAPPER(String, getTransportProfileUri, transportProfileUri)
    UAPP_GETTER(UA_Byte, getSecurityLevel, securityLevel)
};

/* --------------------------------------- Node attributes -------------------------------------- */

/**
 * Node attributes mask.
 * Bitmask used in the node attributes parameters to specify which attributes are set.
 * @see UA_NodeAttributesMask
 */
enum class NodeAttributesMask : uint32_t {
    // clang-format off
    None                    = 0,
    AccessLevel             = 1,
    ArrayDimensions         = 2,
    BrowseName              = 4,
    ContainsNoLoops         = 8,
    DataType                = 16,
    Description             = 32,
    DisplayName             = 64,
    EventNotifier           = 128,
    Executable              = 256,
    Historizing             = 512,
    InverseName             = 1024,
    IsAbstract              = 2048,
    MinimumSamplingInterval = 4096,
    NodeClass               = 8192,
    NodeId                  = 16384,
    Symmetric               = 32768,
    UserAccessLevel         = 65536,
    UserExecutable          = 131072,
    UserWriteMask           = 262144,
    ValueRank               = 524288,
    WriteMask               = 1048576,
    Value                   = 2097152,
    DataTypeDefinition      = 4194304,
    RolePermissions         = 8388608,
    AccessRestrictions      = 16777216,
    All                     = 33554431,
    BaseNode                = 26501220,
    Object                  = 26501348,
    ObjectType              = 26503268,
    Variable                = 26571383,
    VariableType            = 28600438,
    Method                  = 26632548,
    ReferenceType           = 26537060,
    View                    = 26501356,
    // clang-format on
};

template <>
struct IsBitmaskEnum<NodeAttributesMask> : std::true_type {};

// Specifialized macros to generate getters/setters for `UA_*Attribute` classes.
// The `specifiedAttributes` mask is automatically updated in the setter methods.
// A fluent interface is used for the setter methods.

// NOLINTNEXTLINE
#define UAPP_NODEATTR(Type, suffix, member, flag)                                                  \
    UAPP_GETTER(Type, get##suffix, member)                                                         \
    auto& set##suffix(Type member) noexcept {                                                      \
        handle()->specifiedAttributes |= flag;                                                     \
        handle()->member = member;                                                                 \
        return *this;                                                                              \
    }

// NOLINTNEXTLINE
#define UAPP_NODEATTR_BITMASK(Type, suffix, member, flag)                                          \
    UAPP_GETTER(Type, get##suffix, member)                                                         \
    auto& set##suffix(Type member) noexcept {                                                      \
        handle()->specifiedAttributes |= flag;                                                     \
        handle()->member = member.get();                                                           \
        return *this;                                                                              \
    }

// NOLINTNEXTLINE
#define UAPP_NODEATTR_CAST(Type, suffix, member, flag)                                             \
    UAPP_GETTER_CAST(Type, get##suffix, member)                                                    \
    auto& set##suffix(Type member) noexcept {                                                      \
        handle()->specifiedAttributes |= flag;                                                     \
        handle()->member = static_cast<decltype(handle()->member)>(member);                        \
        return *this;                                                                              \
    }

// NOLINTNEXTLINE
#define UAPP_NODEATTR_WRAPPER(Type, suffix, member, flag)                                          \
    UAPP_GETTER_WRAPPER_CONST(Type, get##suffix, member)                                           \
    auto& set##suffix(const Type& member) {                                                        \
        handle()->specifiedAttributes |= flag;                                                     \
        asWrapper<Type>(handle()->member) = member;                                                \
        return *this;                                                                              \
    }

// NOLINTNEXTLINE
#define UAPP_NODEATTR_ARRAY(Type, suffix, member, memberSize, flag)                                \
    UAPP_GETTER_SPAN(Type, get##suffix, member, memberSize)                                        \
    auto& set##suffix(Span<const Type> member) {                                                   \
        const auto& dataType = opcua::getDataType<Type>();                                         \
        handle()->specifiedAttributes |= flag;                                                     \
        detail::deallocateArray(handle()->member, handle()->memberSize, dataType);                 \
        handle()->member = detail::copyArray(member.data(), member.size(), dataType);              \
        handle()->memberSize = member.size();                                                      \
        return *this;                                                                              \
    }

// NOLINTNEXTLINT
#define UAPP_NODEATTR_COMMON                                                                       \
    UAPP_GETTER(Bitmask<NodeAttributesMask>, getSpecifiedAttributes, specifiedAttributes)          \
    UAPP_NODEATTR_WRAPPER(                                                                         \
        LocalizedText, DisplayName, displayName, UA_NODEATTRIBUTESMASK_DISPLAYNAME                 \
    )                                                                                              \
    UAPP_NODEATTR_WRAPPER(                                                                         \
        LocalizedText, Description, description, UA_NODEATTRIBUTESMASK_DESCRIPTION                 \
    )                                                                                              \
    UAPP_NODEATTR_BITMASK(                                                                         \
        Bitmask<WriteMask>, WriteMask, writeMask, UA_NODEATTRIBUTESMASK_WRITEMASK                  \
    )                                                                                              \
    UAPP_NODEATTR_BITMASK(                                                                         \
        Bitmask<WriteMask>, UserWriteMask, userWriteMask, UA_NODEATTRIBUTESMASK_USERWRITEMASK      \
    )

/**
 * UA_NodeAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24
 */
class NodeAttributes : public TypeWrapper<UA_NodeAttributes, UA_TYPES_NODEATTRIBUTES> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_NODEATTR_COMMON
};

/**
 * UA_ObjectAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.2
 */
class ObjectAttributes : public TypeWrapper<UA_ObjectAttributes, UA_TYPES_OBJECTATTRIBUTES> {
public:
    using TypeWrapper::TypeWrapper;

    /// Construct with default attribute definitions.
    ObjectAttributes()
        : TypeWrapper(UA_ObjectAttributes_default) {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR_BITMASK(
        Bitmask<EventNotifier>, EventNotifier, eventNotifier, UA_NODEATTRIBUTESMASK_EVENTNOTIFIER
    )
};

/**
 * UA_VariableAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.3
 */
class VariableAttributes : public TypeWrapper<UA_VariableAttributes, UA_TYPES_VARIABLEATTRIBUTES> {
public:
    using TypeWrapper::TypeWrapper;

    /// Construct with default attribute definitions.
    VariableAttributes()
        : TypeWrapper(UA_VariableAttributes_default) {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR_WRAPPER(Variant, Value, value, UA_NODEATTRIBUTESMASK_VALUE)

    /// @see Variant::fromScalar
    template <typename... Args>
    auto& setValueScalar(Args&&... args) {
        return setValue(Variant::fromScalar(std::forward<Args>(args)...));
    }

    /// @see Variant::fromArray
    template <typename... Args>
    auto& setValueArray(Args&&... args) {
        return setValue(Variant::fromArray(std::forward<Args>(args)...));
    }

    UAPP_NODEATTR_WRAPPER(NodeId, DataType, dataType, UA_NODEATTRIBUTESMASK_DATATYPE)

    /// @overload
    /// Deduce the `dataType` from the template type.
    template <typename T>
    auto& setDataType() {
        return setDataType(asWrapper<NodeId>(opcua::getDataType<T>().typeId));
    }

    UAPP_NODEATTR_CAST(ValueRank, ValueRank, valueRank, UA_NODEATTRIBUTESMASK_VALUERANK)
    UAPP_NODEATTR_ARRAY(
        uint32_t,
        ArrayDimensions,
        arrayDimensions,
        arrayDimensionsSize,
        UA_NODEATTRIBUTESMASK_ARRAYDIMENSIONS
    )
    UAPP_NODEATTR_BITMASK(
        Bitmask<AccessLevel>, AccessLevel, accessLevel, UA_NODEATTRIBUTESMASK_ACCESSLEVEL
    )
    UAPP_NODEATTR_BITMASK(
        Bitmask<AccessLevel>,
        UserAccessLevel,
        userAccessLevel,
        UA_NODEATTRIBUTESMASK_USERACCESSLEVEL
    )
    UAPP_NODEATTR(
        double,
        MinimumSamplingInterval,
        minimumSamplingInterval,
        UA_NODEATTRIBUTESMASK_MINIMUMSAMPLINGINTERVAL
    )
    UAPP_NODEATTR(bool, Historizing, historizing, UA_NODEATTRIBUTESMASK_HISTORIZING)
};

/**
 * UA_MethodAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.4
 */
class MethodAttributes : public TypeWrapper<UA_MethodAttributes, UA_TYPES_METHODATTRIBUTES> {
public:
    using TypeWrapper::TypeWrapper;

    /// Construct with default attribute definitions.
    MethodAttributes()
        : TypeWrapper(UA_MethodAttributes_default) {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, Executable, executable, UA_NODEATTRIBUTESMASK_EXECUTABLE)
    UAPP_NODEATTR(bool, UserExecutable, userExecutable, UA_NODEATTRIBUTESMASK_USEREXECUTABLE)
};

/**
 * UA_ObjectTypeAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.5
 */
class ObjectTypeAttributes
    : public TypeWrapper<UA_ObjectTypeAttributes, UA_TYPES_OBJECTTYPEATTRIBUTES> {
public:
    using TypeWrapper::TypeWrapper;

    /// Construct with default attribute definitions.
    ObjectTypeAttributes()
        : TypeWrapper(UA_ObjectTypeAttributes_default) {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, IsAbstract, isAbstract, UA_NODEATTRIBUTESMASK_ISABSTRACT)
};

/**
 * UA_VariableAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.6
 */
class VariableTypeAttributes
    : public TypeWrapper<UA_VariableTypeAttributes, UA_TYPES_VARIABLETYPEATTRIBUTES> {
public:
    using TypeWrapper::TypeWrapper;

    /// Construct with default attribute definitions.
    VariableTypeAttributes()
        : TypeWrapper(UA_VariableTypeAttributes_default) {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR_WRAPPER(Variant, Value, value, UA_NODEATTRIBUTESMASK_VALUE)

    /// @see Variant::fromScalar
    template <typename... Args>
    auto& setValueScalar(Args&&... args) {
        return setValue(Variant::fromScalar(std::forward<Args>(args)...));
    }

    /// @see Variant::fromArray
    template <typename... Args>
    auto& setValueArray(Args&&... args) {
        return setValue(Variant::fromArray(std::forward<Args>(args)...));
    }

    UAPP_NODEATTR_WRAPPER(NodeId, DataType, dataType, UA_NODEATTRIBUTESMASK_DATATYPE)

    /// @overload
    /// Deduce the `dataType` from the template type.
    template <typename T>
    auto& setDataType() {
        return setDataType(asWrapper<NodeId>(opcua::getDataType<T>().typeId));
    }

    UAPP_NODEATTR_CAST(ValueRank, ValueRank, valueRank, UA_NODEATTRIBUTESMASK_VALUERANK)
    UAPP_NODEATTR_ARRAY(
        uint32_t,
        ArrayDimensions,
        arrayDimensions,
        arrayDimensionsSize,
        UA_NODEATTRIBUTESMASK_ARRAYDIMENSIONS
    )
    UAPP_NODEATTR(bool, IsAbstract, isAbstract, UA_NODEATTRIBUTESMASK_ISABSTRACT)
};

/**
 * UA_ReferenceTypeAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.7
 */
class ReferenceTypeAttributes
    : public TypeWrapper<UA_ReferenceTypeAttributes, UA_TYPES_REFERENCETYPEATTRIBUTES> {
public:
    using TypeWrapper::TypeWrapper;

    /// Construct with default attribute definitions.
    ReferenceTypeAttributes()
        : TypeWrapper(UA_ReferenceTypeAttributes_default) {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, IsAbstract, isAbstract, UA_NODEATTRIBUTESMASK_ISABSTRACT)
    UAPP_NODEATTR(bool, Symmetric, symmetric, UA_NODEATTRIBUTESMASK_SYMMETRIC)
    UAPP_NODEATTR_WRAPPER(
        LocalizedText, InverseName, inverseName, UA_NODEATTRIBUTESMASK_INVERSENAME
    )
};

/**
 * UA_DataTypeAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.8
 */
class DataTypeAttributes : public TypeWrapper<UA_DataTypeAttributes, UA_TYPES_DATATYPEATTRIBUTES> {
public:
    using TypeWrapper::TypeWrapper;

    /// Construct with default attribute definitions.
    DataTypeAttributes()
        : TypeWrapper(UA_DataTypeAttributes_default) {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, IsAbstract, isAbstract, UA_NODEATTRIBUTESMASK_ISABSTRACT)
};

/**
 * UA_ViewAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.9
 */
class ViewAttributes : public TypeWrapper<UA_ViewAttributes, UA_TYPES_VIEWATTRIBUTES> {
public:
    using TypeWrapper::TypeWrapper;

    /// Construct with default attribute definitions.
    ViewAttributes()
        : TypeWrapper(UA_ViewAttributes_default) {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, IsAbstract, containsNoLoops, UA_NODEATTRIBUTESMASK_CONTAINSNOLOOPS)
    UAPP_NODEATTR_BITMASK(
        Bitmask<EventNotifier>, EventNotifier, eventNotifier, UA_NODEATTRIBUTESMASK_EVENTNOTIFIER
    )
};

#undef UAPP_NODEATTR
#undef UAPP_NODEATTR_WRAPPER
#undef UAPP_NODEATTR_ARRAY
#undef UAPP_NODEATTR_COMMON

/* ---------------------------------------------------------------------------------------------- */

/**
 * UA_UserIdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41
 */
class UserIdentityToken : public TypeWrapper<UA_UserIdentityToken, UA_TYPES_USERIDENTITYTOKEN> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(String, getPolicyId, policyId)
};

/**
 * UA_AnonymousIdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41.3
 */
class AnonymousIdentityToken
    : public TypeWrapper<UA_AnonymousIdentityToken, UA_TYPES_ANONYMOUSIDENTITYTOKEN> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(String, getPolicyId, policyId)
};

/**
 * UA_UserNameIdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41.4
 */
class UserNameIdentityToken
    : public TypeWrapper<UA_UserNameIdentityToken, UA_TYPES_USERNAMEIDENTITYTOKEN> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(String, getPolicyId, policyId)
    UAPP_GETTER_WRAPPER(String, getUserName, userName)
    UAPP_GETTER_WRAPPER(ByteString, getPassword, password)
    UAPP_GETTER_WRAPPER(String, getEncryptionAlgorithm, encryptionAlgorithm)
};

/**
 * UA_X509IdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41.5
 */
class X509IdentityToken : public TypeWrapper<UA_X509IdentityToken, UA_TYPES_X509IDENTITYTOKEN> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(String, getPolicyId, policyId)
    UAPP_GETTER_WRAPPER(ByteString, getCertificateData, certificateData)
};

/**
 * UA_IssuedIdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41.6
 */
class IssuedIdentityToken
    : public TypeWrapper<UA_IssuedIdentityToken, UA_TYPES_ISSUEDIDENTITYTOKEN> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(String, getPolicyId, policyId)
    UAPP_GETTER_WRAPPER(ByteString, getTokenData, tokenData)
    UAPP_GETTER_WRAPPER(String, getEncryptionAlgorithm, encryptionAlgorithm)
};

/**
 * UA_AddNodesItem wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.2
 */
class AddNodesItem : public TypeWrapper<UA_AddNodesItem, UA_TYPES_ADDNODESITEM> {
public:
    using TypeWrapper::TypeWrapper;

    AddNodesItem(
        ExpandedNodeId parentNodeId,
        NodeId referenceTypeId,
        ExpandedNodeId requestedNewNodeId,
        QualifiedName browseName,
        NodeClass nodeClass,
        ExtensionObject nodeAttributes,
        ExpandedNodeId typeDefinition
    ) {
        handle()->parentNodeId = detail::toNative(std::move(parentNodeId));
        handle()->referenceTypeId = detail::toNative(std::move(referenceTypeId));
        handle()->requestedNewNodeId = detail::toNative(std::move(requestedNewNodeId));
        handle()->browseName = detail::toNative(std::move(browseName));
        handle()->nodeClass = static_cast<UA_NodeClass>(nodeClass);
        handle()->nodeAttributes = detail::toNative(std::move(nodeAttributes));
        handle()->typeDefinition = detail::toNative(std::move(typeDefinition));
    }

    UAPP_GETTER_WRAPPER(ExpandedNodeId, getParentNodeId, parentNodeId)
    UAPP_GETTER_WRAPPER(NodeId, getReferenceTypeId, referenceTypeId)
    UAPP_GETTER_WRAPPER(ExpandedNodeId, getRequestedNewNodeId, requestedNewNodeId)
    UAPP_GETTER_WRAPPER(QualifiedName, getBrowseName, browseName)
    UAPP_GETTER_CAST(NodeClass, getNodeClass, nodeClass)
    UAPP_GETTER_WRAPPER(ExtensionObject, getNodeAttributes, nodeAttributes)
    UAPP_GETTER_WRAPPER(ExpandedNodeId, getTypeDefinition, typeDefinition)
};

/**
 * UA_AddNodesResult wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.2
 */
class AddNodesResult : public TypeWrapper<UA_AddNodesResult, UA_TYPES_ADDNODESRESULT> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER(StatusCode, getStatusCode, statusCode)
    UAPP_GETTER_WRAPPER(NodeId, getAddedNodeId, addedNodeId)
};

/**
 * UA_AddNodesRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.2
 */
class AddNodesRequest : public TypeWrapper<UA_AddNodesRequest, UA_TYPES_ADDNODESREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    AddNodesRequest(RequestHeader requestHeader, Span<const AddNodesItem> nodesToAdd) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->nodesToAddSize = nodesToAdd.size();
        handle()->nodesToAdd = detail::toNativeArray(nodesToAdd);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(AddNodesItem, getNodesToAdd, nodesToAdd, nodesToAddSize)
};

/**
 * UA_AddNodesResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.2
 */
class AddNodesResponse : public TypeWrapper<UA_AddNodesResponse, UA_TYPES_ADDNODESRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(AddNodesResult, getResults, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_AddReferencesItem wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.3
 */
class AddReferencesItem : public TypeWrapper<UA_AddReferencesItem, UA_TYPES_ADDREFERENCESITEM> {
public:
    using TypeWrapper::TypeWrapper;

    AddReferencesItem(
        NodeId sourceNodeId,
        NodeId referenceTypeId,
        bool isForward,
        std::string_view targetServerUri,
        ExpandedNodeId targetNodeId,
        NodeClass targetNodeClass
    ) {
        handle()->sourceNodeId = detail::toNative(std::move(sourceNodeId));
        handle()->referenceTypeId = detail::toNative(std::move(referenceTypeId));
        handle()->isForward = isForward;
        handle()->targetServerUri = detail::toNative(targetServerUri);
        handle()->targetNodeId = detail::toNative(std::move(targetNodeId));
        handle()->targetNodeClass = static_cast<UA_NodeClass>(targetNodeClass);
    }

    UAPP_GETTER_WRAPPER(NodeId, getSourceNodeId, sourceNodeId)
    UAPP_GETTER_WRAPPER(NodeId, getReferenceTypeId, referenceTypeId)
    UAPP_GETTER(bool, getIsForward, isForward)
    UAPP_GETTER_WRAPPER(String, getTargetServerUri, targetServerUri)
    UAPP_GETTER_WRAPPER(ExpandedNodeId, getTargetNodeId, targetNodeId)
    UAPP_GETTER_CAST(NodeClass, getTargetNodeClass, targetNodeClass)
};

/**
 * UA_AddReferencesRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.3
 */
class AddReferencesRequest
    : public TypeWrapper<UA_AddReferencesRequest, UA_TYPES_ADDREFERENCESREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    AddReferencesRequest(
        RequestHeader requestHeader, Span<const AddReferencesItem> referencesToAdd
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->referencesToAddSize = referencesToAdd.size();
        handle()->referencesToAdd = detail::toNativeArray(referencesToAdd);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(
        AddReferencesItem, getReferencesToAdd, referencesToAdd, referencesToAddSize
    )
};

/**
 * UA_AddReferencesResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.3
 */
class AddReferencesResponse
    : public TypeWrapper<UA_AddReferencesResponse, UA_TYPES_ADDREFERENCESRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, getResults, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_DeleteNodesItem wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.4
 */
class DeleteNodesItem : public TypeWrapper<UA_DeleteNodesItem, UA_TYPES_DELETENODESITEM> {
public:
    using TypeWrapper::TypeWrapper;

    DeleteNodesItem(NodeId nodeId, bool deleteTargetReferences) {
        handle()->nodeId = detail::toNative(std::move(nodeId));
        handle()->deleteTargetReferences = deleteTargetReferences;
    }

    UAPP_GETTER_WRAPPER(NodeId, getNodeId, nodeId)
    UAPP_GETTER(bool, getDeleteTargetReferences, deleteTargetReferences)
};

/**
 * UA_DeleteNodesRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.4
 */
class DeleteNodesRequest : public TypeWrapper<UA_DeleteNodesRequest, UA_TYPES_DELETENODESREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    DeleteNodesRequest(RequestHeader requestHeader, Span<const DeleteNodesItem> nodesToDelete) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->nodesToDeleteSize = nodesToDelete.size();
        handle()->nodesToDelete = detail::toNativeArray(nodesToDelete);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(DeleteNodesItem, getNodesToDelete, nodesToDelete, nodesToDeleteSize)
};

/**
 * UA_DeleteNodesResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.4
 */
class DeleteNodesResponse
    : public TypeWrapper<UA_DeleteNodesResponse, UA_TYPES_DELETENODESRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, getResults, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_DeleteReferencesItem wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.5
 */
class DeleteReferencesItem
    : public TypeWrapper<UA_DeleteReferencesItem, UA_TYPES_DELETEREFERENCESITEM> {
public:
    using TypeWrapper::TypeWrapper;

    DeleteReferencesItem(
        NodeId sourceNodeId,
        NodeId referenceTypeId,
        bool isForward,
        ExpandedNodeId targetNodeId,
        bool deleteBidirectional
    ) {
        handle()->sourceNodeId = detail::toNative(std::move(sourceNodeId));
        handle()->referenceTypeId = detail::toNative(std::move(referenceTypeId));
        handle()->isForward = isForward;
        handle()->targetNodeId = detail::toNative(std::move(targetNodeId));
        handle()->deleteBidirectional = deleteBidirectional;
    }

    UAPP_GETTER_WRAPPER(NodeId, getSourceNodeId, sourceNodeId)
    UAPP_GETTER_WRAPPER(NodeId, getReferenceTypeId, referenceTypeId)
    UAPP_GETTER(bool, getIsForward, isForward)
    UAPP_GETTER_WRAPPER(ExpandedNodeId, getTargetNodeId, targetNodeId)
    UAPP_GETTER(bool, getDeleteBidirectional, deleteBidirectional)
};

/**
 * UA_DeleteReferencesRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.5
 */
class DeleteReferencesRequest
    : public TypeWrapper<UA_DeleteReferencesRequest, UA_TYPES_DELETEREFERENCESREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    DeleteReferencesRequest(
        RequestHeader requestHeader, Span<const DeleteReferencesItem> referencesToDelete
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->referencesToDeleteSize = referencesToDelete.size();
        handle()->referencesToDelete = detail::toNativeArray(referencesToDelete);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(
        DeleteReferencesItem, getReferencesToDelete, referencesToDelete, referencesToDeleteSize
    )
};

/**
 * UA_DeleteReferencesResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.5
 */
class DeleteReferencesResponse
    : public TypeWrapper<UA_DeleteReferencesResponse, UA_TYPES_DELETEREFERENCESRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, getResults, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_ViewDescription wrapper class.
 */
class ViewDescription : public TypeWrapper<UA_ViewDescription, UA_TYPES_VIEWDESCRIPTION> {
public:
    using TypeWrapper::TypeWrapper;

    ViewDescription(NodeId viewId, DateTime timestamp, uint32_t viewVersion) {
        handle()->viewId = detail::toNative(std::move(viewId));
        handle()->timestamp = timestamp;
        handle()->viewVersion = viewVersion;
    }

    UAPP_GETTER_WRAPPER(NodeId, getViewId, viewId)
    UAPP_GETTER_WRAPPER(DateTime, getTimestamp, timestamp)
    UAPP_GETTER(uint32_t, getViewVersion, viewVersion)
};

/**
 * Browse result mask.
 *
 * The enum can be used as a bitmask and allows bitwise operations, e.g.:
 * @code
 * auto mask = BrowseResultMask::ReferenceTypeId | BrowseResultMask::IsForward;
 * @endcode
 *
 * @see UA_BrowseResultMask
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.2
 */
enum class BrowseResultMask : uint32_t {
    // clang-format off
    None              = UA_BROWSERESULTMASK_NONE,
    ReferenceTypeId   = UA_BROWSERESULTMASK_REFERENCETYPEID,
    IsForward         = UA_BROWSERESULTMASK_ISFORWARD,
    NodeClass         = UA_BROWSERESULTMASK_NODECLASS,
    BrowseName        = UA_BROWSERESULTMASK_BROWSENAME,
    DisplayName       = UA_BROWSERESULTMASK_DISPLAYNAME,
    TypeDefinition    = UA_BROWSERESULTMASK_TYPEDEFINITION,
    All               = UA_BROWSERESULTMASK_ALL,
    ReferenceTypeInfo = UA_BROWSERESULTMASK_REFERENCETYPEINFO,
    TargetInfo        = UA_BROWSERESULTMASK_TARGETINFO,
    // clang-format on
};

template <>
struct IsBitmaskEnum<BrowseResultMask> : std::true_type {};

/**
 * UA_BrowseDescription wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.2
 */
class BrowseDescription : public TypeWrapper<UA_BrowseDescription, UA_TYPES_BROWSEDESCRIPTION> {
public:
    using TypeWrapper::TypeWrapper;

    BrowseDescription(
        NodeId nodeId,
        BrowseDirection browseDirection,
        NodeId referenceTypeId = ReferenceTypeId::References,
        bool includeSubtypes = true,
        Bitmask<NodeClass> nodeClassMask = NodeClass::Unspecified,
        Bitmask<BrowseResultMask> resultMask = BrowseResultMask::All
    ) {
        handle()->nodeId = detail::toNative(std::move(nodeId));
        handle()->browseDirection = static_cast<UA_BrowseDirection>(browseDirection);
        handle()->referenceTypeId = detail::toNative(std::move(referenceTypeId));
        handle()->includeSubtypes = includeSubtypes;
        handle()->nodeClassMask = nodeClassMask.get();
        handle()->resultMask = resultMask.get();
    }

    UAPP_GETTER_WRAPPER(NodeId, getNodeId, nodeId)
    UAPP_GETTER_CAST(BrowseDirection, getBrowseDirection, browseDirection)
    UAPP_GETTER_WRAPPER(NodeId, getReferenceTypeId, referenceTypeId)
    UAPP_GETTER(bool, getIncludeSubtypes, includeSubtypes)
    UAPP_GETTER(Bitmask<NodeClass>, getNodeClassMask, nodeClassMask)
    UAPP_GETTER(Bitmask<BrowseResultMask>, getResultMask, resultMask)
};

/**
 * UA_BrowseRequest wrapper class.
 */
class BrowseRequest : public TypeWrapper<UA_BrowseRequest, UA_TYPES_BROWSEREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    BrowseRequest(
        RequestHeader requestHeader,
        ViewDescription view,
        uint32_t requestedMaxReferencesPerNode,
        Span<const BrowseDescription> nodesToBrowse
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->view = detail::toNative(std::move(view));
        handle()->requestedMaxReferencesPerNode = requestedMaxReferencesPerNode;
        handle()->nodesToBrowseSize = nodesToBrowse.size();
        handle()->nodesToBrowse = detail::toNativeArray(nodesToBrowse);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER_WRAPPER(ViewDescription, getView, view)
    UAPP_GETTER(uint32_t, getRequestedMaxReferencesPerNode, requestedMaxReferencesPerNode)
    UAPP_GETTER_SPAN_WRAPPER(BrowseDescription, getNodesToBrowse, nodesToBrowse, nodesToBrowseSize)
};

/**
 * UA_ReferenceDescription wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.30
 */
class ReferenceDescription
    : public TypeWrapper<UA_ReferenceDescription, UA_TYPES_REFERENCEDESCRIPTION> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(NodeId, getReferenceTypeId, referenceTypeId)
    UAPP_GETTER(bool, getIsForward, isForward)
    UAPP_GETTER_WRAPPER(ExpandedNodeId, getNodeId, nodeId)
    UAPP_GETTER_WRAPPER(QualifiedName, getBrowseName, browseName)
    UAPP_GETTER_WRAPPER(LocalizedText, getDisplayName, displayName)
    UAPP_GETTER_CAST(NodeClass, getNodeClass, nodeClass)
    UAPP_GETTER_WRAPPER(ExpandedNodeId, getTypeDefinition, typeDefinition)
};

/**
 * UA_BrowseResult wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.6
 */
class BrowseResult : public TypeWrapper<UA_BrowseResult, UA_TYPES_BROWSERESULT> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER(StatusCode, getStatusCode, statusCode)
    UAPP_GETTER_WRAPPER(ByteString, getContinuationPoint, continuationPoint)
    UAPP_GETTER_SPAN_WRAPPER(ReferenceDescription, getReferences, references, referencesSize)
};

/**
 * UA_BrowseResponse wrapper class.
 */
class BrowseResponse : public TypeWrapper<UA_BrowseResponse, UA_TYPES_BROWSERESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(BrowseResult, getResults, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_BrowseNextRequest wrapper class.
 */
class BrowseNextRequest : public TypeWrapper<UA_BrowseNextRequest, UA_TYPES_BROWSENEXTREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    BrowseNextRequest(
        RequestHeader requestHeader,
        bool releaseContinuationPoints,
        Span<const ByteString> continuationPoints
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->releaseContinuationPoints = releaseContinuationPoints;
        handle()->continuationPointsSize = continuationPoints.size();
        handle()->continuationPoints = detail::toNativeArray(continuationPoints);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER(bool, getReleaseContinuationPoints, releaseContinuationPoints)
    UAPP_GETTER_SPAN_WRAPPER(
        ByteString, getContinuationPoints, continuationPoints, continuationPointsSize
    )
};

/**
 * UA_BrowseNextResponse wrapper class.
 */
class BrowseNextResponse : public TypeWrapper<UA_BrowseNextResponse, UA_TYPES_BROWSENEXTRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(BrowseResult, getResults, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_RelativePathElement wrapper class.
 */
class RelativePathElement
    : public TypeWrapper<UA_RelativePathElement, UA_TYPES_RELATIVEPATHELEMENT> {
public:
    using TypeWrapper::TypeWrapper;

    RelativePathElement(
        NodeId referenceTypeId, bool isInverse, bool includeSubtypes, QualifiedName targetName
    ) {
        handle()->referenceTypeId = detail::toNative(std::move(referenceTypeId));
        handle()->isInverse = isInverse;
        handle()->includeSubtypes = includeSubtypes;
        handle()->targetName = detail::toNative(std::move(targetName));
    }

    UAPP_GETTER_WRAPPER(NodeId, getReferenceTypeId, referenceTypeId)
    UAPP_GETTER(bool, getIsInverse, isInverse)
    UAPP_GETTER(bool, getIncludeSubtypes, includeSubtypes)
    UAPP_GETTER_WRAPPER(QualifiedName, getTargetName, targetName)
};

/**
 * UA_RelativePath wrapper class.
 */
class RelativePath : public TypeWrapper<UA_RelativePath, UA_TYPES_RELATIVEPATH> {
public:
    using TypeWrapper::TypeWrapper;

    RelativePath(std::initializer_list<RelativePathElement> elements)
        : RelativePath({elements.begin(), elements.size()}) {}

    explicit RelativePath(Span<const RelativePathElement> elements) {
        handle()->elementsSize = elements.size();
        handle()->elements = detail::toNativeArray(elements);
    }

    UAPP_GETTER_SPAN_WRAPPER(RelativePathElement, getElements, elements, elementsSize)
};

/**
 * UA_BrowsePath wrapper class.
 */
class BrowsePath : public TypeWrapper<UA_BrowsePath, UA_TYPES_BROWSEPATH> {
public:
    using TypeWrapper::TypeWrapper;

    BrowsePath(NodeId startingNode, RelativePath relativePath) {
        handle()->startingNode = detail::toNative(std::move(startingNode));
        handle()->relativePath = detail::toNative(std::move(relativePath));
    }

    UAPP_GETTER_WRAPPER(NodeId, getStartingNode, startingNode)
    UAPP_GETTER_WRAPPER(RelativePath, getRelativePath, relativePath)
};

/**
 * UA_BrowsePathTarget wrapper class.
 */
class BrowsePathTarget : public TypeWrapper<UA_BrowsePathTarget, UA_TYPES_BROWSEPATHTARGET> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ExpandedNodeId, getTargetId, targetId)
    UAPP_GETTER(uint32_t, getRemainingPathIndex, remainingPathIndex)
};

/**
 * UA_BrowsePathResult wrapper class.
 */
class BrowsePathResult : public TypeWrapper<UA_BrowsePathResult, UA_TYPES_BROWSEPATHRESULT> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER(StatusCode, getStatusCode, statusCode)
    UAPP_GETTER_SPAN_WRAPPER(BrowsePathTarget, getTargets, targets, targetsSize)
};

/**
 * UA_TranslateBrowsePathsToNodeIdsRequest wrapper class.
 */
class TranslateBrowsePathsToNodeIdsRequest
    : public TypeWrapper<
          UA_TranslateBrowsePathsToNodeIdsRequest,
          UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    TranslateBrowsePathsToNodeIdsRequest(
        RequestHeader requestHeader, Span<const BrowsePath> browsePaths
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->browsePathsSize = browsePaths.size();
        handle()->browsePaths = detail::toNativeArray(browsePaths);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(BrowsePath, getBrowsePaths, browsePaths, browsePathsSize)
};

/**
 * UA_TranslateBrowsePathsToNodeIdsResponse wrapper class.
 */
class TranslateBrowsePathsToNodeIdsResponse
    : public TypeWrapper<
          UA_TranslateBrowsePathsToNodeIdsResponse,
          UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(BrowsePathResult, getResults, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_RegisterNodesRequest wrapper class.
 */
class RegisterNodesRequest
    : public TypeWrapper<UA_RegisterNodesRequest, UA_TYPES_REGISTERNODESREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    RegisterNodesRequest(RequestHeader requestHeader, Span<const NodeId> nodesToRegister) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->nodesToRegisterSize = nodesToRegister.size();
        handle()->nodesToRegister = detail::toNativeArray(nodesToRegister);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(NodeId, getNodesToRegister, nodesToRegister, nodesToRegisterSize)
};

/**
 * UA_RegisterNodesResponse wrapper class.
 */
class RegisterNodesResponse
    : public TypeWrapper<UA_RegisterNodesResponse, UA_TYPES_REGISTERNODESRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(NodeId, getRegisteredNodeIds, registeredNodeIds, registeredNodeIdsSize)
};

/**
 * UA_UnregisterNodesRequest wrapper class.
 */
class UnregisterNodesRequest
    : public TypeWrapper<UA_UnregisterNodesRequest, UA_TYPES_UNREGISTERNODESREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    UnregisterNodesRequest(RequestHeader requestHeader, Span<const NodeId> nodesToUnregister) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->nodesToUnregisterSize = nodesToUnregister.size();
        handle()->nodesToUnregister = detail::toNativeArray(nodesToUnregister);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(NodeId, getNodesToUnregister, nodesToUnregister, nodesToUnregisterSize)
};

/**
 * UA_UnregisterNodesResponse wrapper class.
 */
class UnregisterNodesResponse
    : public TypeWrapper<UA_UnregisterNodesResponse, UA_TYPES_UNREGISTERNODESRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
};

/**
 * UA_ReadValueId wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.29
 */
class ReadValueId : public TypeWrapper<UA_ReadValueId, UA_TYPES_READVALUEID> {
public:
    using TypeWrapper::TypeWrapper;

    ReadValueId(
        NodeId nodeId,
        AttributeId attributeId,
        std::string_view indexRange = {},
        QualifiedName dataEncoding = {}
    ) {
        handle()->nodeId = detail::toNative(std::move(nodeId));
        handle()->attributeId = detail::toNative(attributeId);
        handle()->indexRange = detail::toNative(indexRange);
        handle()->dataEncoding = detail::toNative(std::move(dataEncoding));
    }

    UAPP_GETTER_WRAPPER(NodeId, getNodeId, nodeId)
    UAPP_GETTER_CAST(AttributeId, getAttributeId, attributeId)
    UAPP_GETTER_WRAPPER(String, getIndexRange, indexRange)
    UAPP_GETTER_WRAPPER(QualifiedName, getDataEncoding, dataEncoding)
};

/**
 * UA_ReadRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.2
 */
class ReadRequest : public TypeWrapper<UA_ReadRequest, UA_TYPES_READREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    ReadRequest(
        RequestHeader requestHeader,
        double maxAge,
        TimestampsToReturn timestampsToReturn,
        Span<const ReadValueId> nodesToRead
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->maxAge = maxAge;
        handle()->timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestampsToReturn);
        handle()->nodesToReadSize = nodesToRead.size();
        handle()->nodesToRead = detail::toNativeArray(nodesToRead);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER(double, getMaxAge, maxAge)
    UAPP_GETTER_CAST(TimestampsToReturn, getTimestampsToReturn, timestampsToReturn)
    UAPP_GETTER_SPAN_WRAPPER(ReadValueId, getNodesToRead, nodesToRead, nodesToReadSize)
};

/**
 * UA_ReadResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.2
 */
class ReadResponse : public TypeWrapper<UA_ReadResponse, UA_TYPES_READRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(DataValue, getResults, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_WriteValue wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.4
 */
class WriteValue : public TypeWrapper<UA_WriteValue, UA_TYPES_WRITEVALUE> {
public:
    using TypeWrapper::TypeWrapper;

    WriteValue(
        NodeId nodeId, AttributeId attributeId, std::string_view indexRange, DataValue value
    ) {
        handle()->nodeId = detail::toNative(std::move(nodeId));
        handle()->attributeId = detail::toNative(attributeId);
        handle()->indexRange = detail::toNative(indexRange);
        handle()->value = detail::toNative(std::move(value));
    }

    UAPP_GETTER_WRAPPER(NodeId, getNodeId, nodeId)
    UAPP_GETTER_CAST(AttributeId, getAttributeId, attributeId)
    UAPP_GETTER_WRAPPER(String, getIndexRange, indexRange)
    UAPP_GETTER_WRAPPER(DataValue, getValue, value)
};

/**
 * UA_WriteRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.4
 */
class WriteRequest : public TypeWrapper<UA_WriteRequest, UA_TYPES_WRITEREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    WriteRequest(RequestHeader requestHeader, Span<const WriteValue> nodesToWrite) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->nodesToWriteSize = nodesToWrite.size();
        handle()->nodesToWrite = detail::toNativeArray(nodesToWrite);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(WriteValue, getNodesToWrite, nodesToWrite, nodesToWriteSize)
};

/**
 * UA_WriteResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.4
 */
class WriteResponse : public TypeWrapper<UA_WriteResponse, UA_TYPES_WRITERESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, getResults, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/* ------------------------------------------- Method ------------------------------------------- */

#ifdef UA_ENABLE_METHODCALLS

/**
 * UA_Argument wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.6
 */
class Argument : public TypeWrapper<UA_Argument, UA_TYPES_ARGUMENT> {
public:
    using TypeWrapper::TypeWrapper;

    Argument(
        std::string_view name,
        LocalizedText description,
        NodeId dataType,
        ValueRank valueRank = {},
        Span<const uint32_t> arrayDimensions = {}
    ) {
        handle()->name = detail::toNative(name);
        handle()->description = detail::toNative(std::move(description));
        handle()->dataType = detail::toNative(std::move(dataType));
        handle()->valueRank = detail::toNative(valueRank);
        handle()->arrayDimensionsSize = arrayDimensions.size();
        handle()->arrayDimensions = detail::toNativeArray(arrayDimensions);
    }

    UAPP_GETTER_WRAPPER(String, getName, name)
    UAPP_GETTER_WRAPPER(LocalizedText, getDescription, description)
    UAPP_GETTER_WRAPPER(NodeId, getDataType, dataType)
    UAPP_GETTER_CAST(ValueRank, getValueRank, valueRank)
    UAPP_GETTER_SPAN(uint32_t, getArrayDimensions, arrayDimensions, arrayDimensionsSize)
};

/**
 * UA_CallMethodRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11.2
 */
class CallMethodRequest : public TypeWrapper<UA_CallMethodRequest, UA_TYPES_CALLMETHODREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    CallMethodRequest(NodeId objectId, NodeId methodId, Span<const Variant> inputArguments) {
        handle()->objectId = detail::toNative(std::move(objectId));
        handle()->methodId = detail::toNative(std::move(methodId));
        handle()->inputArgumentsSize = inputArguments.size();
        handle()->inputArguments = detail::toNativeArray(inputArguments);
    }

    UAPP_GETTER_WRAPPER(NodeId, getObjectId, objectId)
    UAPP_GETTER_WRAPPER(NodeId, getMethodId, methodId)
    UAPP_GETTER_SPAN_WRAPPER(Variant, getInputArguments, inputArguments, inputArgumentsSize)
};

/**
 * UA_CallMethodResult wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11.2
 */
class CallMethodResult : public TypeWrapper<UA_CallMethodResult, UA_TYPES_CALLMETHODRESULT> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(StatusCode, getStatusCode, statusCode)
    UAPP_GETTER_SPAN_WRAPPER(
        StatusCode, getInputArgumentResults, inputArgumentResults, inputArgumentResultsSize
    )
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo,
        getInputArgumentDiagnosticInfos,
        inputArgumentDiagnosticInfos,
        inputArgumentDiagnosticInfosSize
    )
    UAPP_GETTER_SPAN_WRAPPER(Variant, getOutputArguments, outputArguments, outputArgumentsSize)
};

/**
 * UA_CallRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11.2
 */
class CallRequest : public TypeWrapper<UA_CallRequest, UA_TYPES_CALLREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    CallRequest(RequestHeader requestHeader, Span<const CallMethodRequest> methodsToCall) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->methodsToCallSize = methodsToCall.size();
        handle()->methodsToCall = detail::toNativeArray(methodsToCall);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(CallMethodRequest, getMethodsToCall, methodsToCall, methodsToCallSize)
};

/**
 * UA_CallResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11.2
 */
class CallResponse : public TypeWrapper<UA_CallResponse, UA_TYPES_CALLRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(CallMethodResult, getResults, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

#endif

/* ---------------------------------------- Subscriptions --------------------------------------- */

#ifdef UA_ENABLE_SUBSCRIPTIONS

/**
 * Filter operator.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.3
 */
enum class FilterOperator : int32_t {
    // clang-format off
    Equals             = 0,
    IsNull             = 1,
    GreaterThan        = 2,
    LessThan           = 3,
    GreaterThanOrEqual = 4,
    LessThanOrEqual    = 5,
    Like               = 6,
    Not                = 7,
    Between            = 8,
    InList             = 9,
    And                = 10,
    Or                 = 11,
    Cast               = 12,
    InView             = 13,
    OfType             = 14,
    RelatedTo          = 15,
    BitwiseAnd         = 16,
    BitwiseOr          = 17,
    // clang-format on
};

/**
 * UA_ElementOperand wrapper class.
 * Reference a sub-element in the ContentFilter elements array by index.
 * The index must be greater than the element index it is part of.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.4.2
 */
class ElementOperand : public TypeWrapper<UA_ElementOperand, UA_TYPES_ELEMENTOPERAND> {
public:
    using TypeWrapper::TypeWrapper;

    explicit ElementOperand(uint32_t index) {
        handle()->index = index;
    }

    UAPP_GETTER(uint32_t, getIndex, index)
};

/**
 * UA_LiteralOperand wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.4.3
 */
class LiteralOperand : public TypeWrapper<UA_LiteralOperand, UA_TYPES_LITERALOPERAND> {
private:
    template <typename T>
    using EnableIfLiteral =
        std::enable_if_t<!detail::IsOneOf<T, Variant, UA_LiteralOperand, LiteralOperand>::value>;

public:
    using TypeWrapper::TypeWrapper;

    explicit LiteralOperand(Variant value) {
        handle()->value = detail::toNative(std::move(value));
    }

    template <typename T, typename = EnableIfLiteral<T>>
    explicit LiteralOperand(T&& literal)
        : LiteralOperand(Variant::fromScalar(std::forward<T>(literal))) {}

    UAPP_GETTER_WRAPPER(Variant, getValue, value)
};

/**
 * UA_AttributeOperand wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.4.4
 */
class AttributeOperand : public TypeWrapper<UA_AttributeOperand, UA_TYPES_ATTRIBUTEOPERAND> {
public:
    using TypeWrapper::TypeWrapper;

    AttributeOperand(
        NodeId nodeId,
        std::string_view alias,
        RelativePath browsePath,
        AttributeId attributeId,
        std::string_view indexRange = {}
    ) {
        handle()->nodeId = detail::toNative(std::move(nodeId));
        handle()->alias = detail::toNative(alias);
        handle()->browsePath = detail::toNative(std::move(browsePath));
        handle()->attributeId = detail::toNative(attributeId);
        handle()->indexRange = detail::toNative(indexRange);
    }

    UAPP_GETTER_WRAPPER(NodeId, getNodeId, nodeId)
    UAPP_GETTER_WRAPPER(String, getAlias, alias)
    UAPP_GETTER_WRAPPER(RelativePath, getBrowsePath, browsePath)
    UAPP_GETTER_CAST(AttributeId, getAttributeId, attributeId)
    UAPP_GETTER_WRAPPER(String, getIndexRange, indexRange)
};

/**
 * UA_SimpleAttributeOperand wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.4.5
 */
class SimpleAttributeOperand
    : public TypeWrapper<UA_SimpleAttributeOperand, UA_TYPES_SIMPLEATTRIBUTEOPERAND> {
public:
    using TypeWrapper::TypeWrapper;

    SimpleAttributeOperand(
        NodeId typeDefinitionId,
        Span<const QualifiedName> browsePath,
        AttributeId attributeId,
        std::string_view indexRange = {}
    ) {
        handle()->typeDefinitionId = detail::toNative(std::move(typeDefinitionId));
        handle()->browsePathSize = browsePath.size();
        handle()->browsePath = detail::toNativeArray(browsePath);
        handle()->attributeId = detail::toNative(attributeId);
        handle()->indexRange = detail::toNative(indexRange);
    }

    UAPP_GETTER_WRAPPER(NodeId, getTypeDefinitionId, typeDefinitionId)
    UAPP_GETTER_SPAN_WRAPPER(QualifiedName, getBrowsePath, browsePath, browsePathSize)
    UAPP_GETTER_CAST(AttributeId, getAttributeId, attributeId)
    UAPP_GETTER_WRAPPER(String, getIndexRange, indexRange)
};

/**
 * Filter operand.
 *
 * The FilterOperand is an extensible parameter and can be of type:
 * - ElementOperand
 * - LiteralOperand
 * - AttributeOperand
 * - SimpleAttributeOperand
 * - ExtensionObject (other types)
 *
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.4
 */
using FilterOperand = std::variant<
    ElementOperand,
    LiteralOperand,
    AttributeOperand,
    SimpleAttributeOperand,
    ExtensionObject>;

/**
 * UA_ContentFilterElement wrapper class.
 *
 * ContentFilterElements compose a filter criteria with an operator and its operands.
 * ContentFilterElements can be composed to ContentFilter objects with the `&&`/`||` operators and
 * negated with the `!` operator.
 *
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.1
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/B.1
 */
class ContentFilterElement
    : public TypeWrapper<UA_ContentFilterElement, UA_TYPES_CONTENTFILTERELEMENT> {
public:
    using TypeWrapper::TypeWrapper;

    ContentFilterElement(FilterOperator filterOperator, Span<const FilterOperand> operands);

    UAPP_GETTER_CAST(FilterOperator, getFilterOperator, filterOperator)
    UAPP_GETTER_SPAN_WRAPPER(ExtensionObject, getFilterOperands, filterOperands, filterOperandsSize)
};

/**
 * UA_ContentFilter wrapper class.
 *
 * A collection of ContentFilterElement objects that define a filter criteria. The ContentFilter has
 * a tree-like structure with references to sub-elements (of type ElementOperand). The evaluation of
 * the ContentFilter starts with the first entry of the elements.
 * ContentFilter objects can be composed with `&&`/`||` operators and negated with the `!` operator.
 *
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.1
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/B.1
 */
class ContentFilter : public TypeWrapper<UA_ContentFilter, UA_TYPES_CONTENTFILTER> {
public:
    using TypeWrapper::TypeWrapper;

    ContentFilter(std::initializer_list<ContentFilterElement> elements);
    explicit ContentFilter(Span<const ContentFilterElement> elements);

    UAPP_GETTER_SPAN_WRAPPER(ContentFilterElement, getElements, elements, elementsSize)
};

ContentFilter operator!(const ContentFilterElement& filterElement);
ContentFilter operator!(const ContentFilter& filter);

ContentFilter operator&&(const ContentFilterElement& lhs, const ContentFilterElement& rhs);
ContentFilter operator&&(const ContentFilterElement& lhs, const ContentFilter& rhs);
ContentFilter operator&&(const ContentFilter& lhs, const ContentFilterElement& rhs);
ContentFilter operator&&(const ContentFilter& lhs, const ContentFilter& rhs);

ContentFilter operator||(const ContentFilterElement& lhs, const ContentFilterElement& rhs);
ContentFilter operator||(const ContentFilterElement& lhs, const ContentFilter& rhs);
ContentFilter operator||(const ContentFilter& lhs, const ContentFilterElement& rhs);
ContentFilter operator||(const ContentFilter& lhs, const ContentFilter& rhs);

/**
 * Data change trigger.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.10
 */
enum class DataChangeTrigger : int32_t {
    // clang-format off
    Status               = 0,
    StatusValue          = 1,
    StatusValueTimestamp = 2,
    // clang-format on
};

/**
 * Deadband type.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.22.2
 */
enum class DeadbandType : int32_t {
    // clang-format off
    None     = 0,
    Absolute = 1,
    Percent  = 2,
    // clang-format on
};

/**
 * UA_DataChangeFilter wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.22.2
 */
class DataChangeFilter : public TypeWrapper<UA_DataChangeFilter, UA_TYPES_DATACHANGEFILTER> {
public:
    using TypeWrapper::TypeWrapper;

    DataChangeFilter(DataChangeTrigger trigger, DeadbandType deadbandType, double deadbandValue) {
        handle()->trigger = static_cast<UA_DataChangeTrigger>(trigger);
        handle()->deadbandType = detail::toNative(deadbandType);
        handle()->deadbandValue = deadbandValue;
    }

    UAPP_GETTER_CAST(DataChangeTrigger, getTrigger, trigger)
    UAPP_GETTER_CAST(DeadbandType, getDeadbandType, deadbandType)
    UAPP_GETTER(double, getDeadbandValue, deadbandValue)
};

/**
 * UA_EventFilter wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.22.3
 */
class EventFilter : public TypeWrapper<UA_EventFilter, UA_TYPES_EVENTFILTER> {
public:
    using TypeWrapper::TypeWrapper;

    EventFilter(Span<const SimpleAttributeOperand> selectClauses, ContentFilter whereClause) {
        handle()->selectClausesSize = selectClauses.size();
        handle()->selectClauses = detail::toNativeArray(selectClauses);
        handle()->whereClause = detail::toNative(std::move(whereClause));
    }

    UAPP_GETTER_SPAN_WRAPPER(
        SimpleAttributeOperand, getSelectClauses, selectClauses, selectClausesSize
    )
    UAPP_GETTER_WRAPPER(ContentFilter, getWhereClause, whereClause)
};

using AggregateConfiguration = UA_AggregateConfiguration;

/**
 * UA_AggregateFilter wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.22.4
 */
class AggregateFilter : public TypeWrapper<UA_AggregateFilter, UA_TYPES_AGGREGATEFILTER> {
public:
    using TypeWrapper::TypeWrapper;

    AggregateFilter(
        DateTime startTime,
        NodeId aggregateType,
        double processingInterval,
        AggregateConfiguration aggregateConfiguration
    ) {
        handle()->startTime = detail::toNative(std::move(startTime));
        handle()->aggregateType = detail::toNative(std::move(aggregateType));
        handle()->processingInterval = processingInterval;
        handle()->aggregateConfiguration = aggregateConfiguration;  // TODO: make wrapper?
    }

    UAPP_GETTER_WRAPPER(DateTime, getStartTime, startTime)
    UAPP_GETTER_WRAPPER(NodeId, getAggregateType, aggregateType)
    UAPP_GETTER(double, getProcessingInterval, processingInterval)
    UAPP_GETTER(AggregateConfiguration, getAggregateConfiguration, aggregateConfiguration)
};

/**
 * UA_MonitoringParameters wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.21
 */
class MonitoringParameters
    : public TypeWrapper<UA_MonitoringParameters, UA_TYPES_MONITORINGPARAMETERS> {
public:
    using TypeWrapper::TypeWrapper;

    /// Construct with default values from open62541.
    /// The `clientHandle` parameter cannot be set by the user, any value will be replaced by the
    /// client before sending the request to the server.
    // NOLINTNEXTLINE
    MonitoringParameters(
        double samplingInterval = 250.0,
        ExtensionObject filter = {},
        uint32_t queueSize = 1U,
        bool discardOldest = true
    ) {
        handle()->samplingInterval = samplingInterval;
        handle()->filter = detail::toNative(std::move(filter));
        handle()->queueSize = queueSize;
        handle()->discardOldest = discardOldest;
    }

    UAPP_GETTER(double, getSamplingInterval, samplingInterval)
    UAPP_GETTER_WRAPPER(ExtensionObject, getFilter, filter)
    UAPP_GETTER(uint32_t, getQueueSize, queueSize)
    UAPP_GETTER(bool, getDiscardOldest, discardOldest)
};

/**
 * UA_MonitoredItemCreateRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.2
 */
class MonitoredItemCreateRequest
    : public TypeWrapper<UA_MonitoredItemCreateRequest, UA_TYPES_MONITOREDITEMCREATEREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    explicit MonitoredItemCreateRequest(
        ReadValueId itemToMonitor,
        MonitoringMode monitoringMode = MonitoringMode::Reporting,
        MonitoringParameters requestedParameters = {}
    ) {
        handle()->itemToMonitor = detail::toNative(std::move(itemToMonitor));
        handle()->monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
        handle()->requestedParameters = detail::toNative(std::move(requestedParameters));
    }

    UAPP_GETTER_WRAPPER(ReadValueId, getItemToMonitor, itemToMonitor)
    UAPP_GETTER_CAST(MonitoringMode, getMonitoringMode, monitoringMode)
    UAPP_GETTER_WRAPPER(MonitoringParameters, getRequestedParameters, requestedParameters)
};

/**
 * UA_MonitoredItemCreateResult wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.2
 */
class MonitoredItemCreateResult
    : public TypeWrapper<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(StatusCode, getStatusCode, statusCode);
    UAPP_GETTER(uint32_t, getMonitoredItemId, monitoredItemId);
    UAPP_GETTER(double, getRevisedSamplingInterval, revisedSamplingInterval);
    UAPP_GETTER(uint32_t, getRevisedQueueSize, revisedQueueSize);
    UAPP_GETTER_WRAPPER(ExtensionObject, getFilterResult, filterResult);
};

/**
 * UA_CreateMonitoredItemsRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.2
 */
class CreateMonitoredItemsRequest
    : public TypeWrapper<UA_CreateMonitoredItemsRequest, UA_TYPES_CREATEMONITOREDITEMSREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    CreateMonitoredItemsRequest(
        RequestHeader requestHeader,
        uint32_t subscriptionId,
        TimestampsToReturn timestampsToReturn,
        Span<const MonitoredItemCreateRequest> itemsToCreate
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->subscriptionId = subscriptionId;
        handle()->timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestampsToReturn);
        handle()->itemsToCreateSize = itemsToCreate.size();
        handle()->itemsToCreate = detail::toNativeArray(itemsToCreate);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER(uint32_t, getSubscriptionId, subscriptionId)
    UAPP_GETTER_CAST(TimestampsToReturn, getTimestampsToReturn, timestampsToReturn)
    UAPP_GETTER_SPAN_WRAPPER(
        MonitoredItemCreateRequest, getItemsToCreate, itemsToCreate, itemsToCreateSize
    )
};

/**
 * UA_CreateMonitoredItemsResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.2
 */
class CreateMonitoredItemsResponse
    : public TypeWrapper<UA_CreateMonitoredItemsResponse, UA_TYPES_CREATEMONITOREDITEMSRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(MonitoredItemCreateResult, getResults, results, resultsSize);
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_MonitoredItemModifyRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.3
 */
class MonitoredItemModifyRequest
    : public TypeWrapper<UA_MonitoredItemModifyRequest, UA_TYPES_MONITOREDITEMMODIFYREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    MonitoredItemModifyRequest(uint32_t monitoredItemId, MonitoringParameters requestedParameters) {
        handle()->monitoredItemId = monitoredItemId;
        handle()->requestedParameters = detail::toNative(std::move(requestedParameters));
    }

    UAPP_GETTER(uint32_t, getMonitoredItemId, monitoredItemId);
    UAPP_GETTER_WRAPPER(MonitoringParameters, getRequestedParameters, requestedParameters)
};

/**
 * UA_MonitoredItemModifyResult wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.3
 */
class MonitoredItemModifyResult
    : public TypeWrapper<UA_MonitoredItemModifyResult, UA_TYPES_MONITOREDITEMMODIFYRESULT> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(StatusCode, getStatusCode, statusCode);
    UAPP_GETTER(double, getRevisedSamplingInterval, revisedSamplingInterval);
    UAPP_GETTER(uint32_t, getRevisedQueueSize, revisedQueueSize);
    UAPP_GETTER_WRAPPER(ExtensionObject, getFilterResult, filterResult);
};

/**
 * UA_ModifyMonitoredItemsRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.3
 */
class ModifyMonitoredItemsRequest
    : public TypeWrapper<UA_ModifyMonitoredItemsRequest, UA_TYPES_MODIFYMONITOREDITEMSREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    ModifyMonitoredItemsRequest(
        RequestHeader requestHeader,
        uint32_t subscriptionId,
        TimestampsToReturn timestampsToReturn,
        Span<const MonitoredItemModifyRequest> itemsToModify
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->subscriptionId = subscriptionId;
        handle()->timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestampsToReturn);
        handle()->itemsToModifySize = itemsToModify.size();
        handle()->itemsToModify = detail::toNativeArray(itemsToModify);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER(uint32_t, getSubscriptionId, subscriptionId)
    UAPP_GETTER_CAST(TimestampsToReturn, getTimestampsToReturn, timestampsToReturn)
    UAPP_GETTER_SPAN_WRAPPER(
        MonitoredItemModifyRequest, getItemsToModify, itemsToModify, itemsToModifySize
    )
};

/**
 * UA_CreateMonitoredItemsResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.3
 */
class ModifyMonitoredItemsResponse
    : public TypeWrapper<UA_ModifyMonitoredItemsResponse, UA_TYPES_MODIFYMONITOREDITEMSRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(MonitoredItemModifyResult, getResults, results, resultsSize);
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_SetMonitoringModeRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.4
 */
class SetMonitoringModeRequest
    : public TypeWrapper<UA_SetMonitoringModeRequest, UA_TYPES_SETMONITORINGMODEREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    SetMonitoringModeRequest(
        RequestHeader requestHeader,
        uint32_t subscriptionId,
        MonitoringMode monitoringMode,
        Span<const uint32_t> monitoredItemIds
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->subscriptionId = subscriptionId;
        handle()->monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
        handle()->monitoredItemIdsSize = monitoredItemIds.size();
        handle()->monitoredItemIds = detail::toNativeArray(monitoredItemIds);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER(uint32_t, getSubscriptionId, subscriptionId)
    UAPP_GETTER_CAST(MonitoringMode, getMonitoringMode, monitoringMode)
    UAPP_GETTER_SPAN(uint32_t, getMonitoredItemIds, monitoredItemIds, monitoredItemIdsSize)
};

/**
 * UA_SetMonitoringModeResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.4
 */
class SetMonitoringModeResponse
    : public TypeWrapper<UA_SetMonitoringModeResponse, UA_TYPES_SETMONITORINGMODERESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, getResults, results, resultsSize);
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_SetTriggeringRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.5
 */
class SetTriggeringRequest
    : public TypeWrapper<UA_SetTriggeringRequest, UA_TYPES_SETTRIGGERINGREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    SetTriggeringRequest(
        RequestHeader requestHeader,
        uint32_t subscriptionId,
        uint32_t triggeringItemId,
        Span<const uint32_t> linksToAdd,
        Span<const uint32_t> linksToRemove
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->subscriptionId = subscriptionId;
        handle()->triggeringItemId = triggeringItemId;
        handle()->linksToAddSize = linksToAdd.size();
        handle()->linksToAdd = detail::toNativeArray(linksToAdd);
        handle()->linksToRemoveSize = linksToRemove.size();
        handle()->linksToRemove = detail::toNativeArray(linksToRemove);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER(uint32_t, getSubscriptionId, subscriptionId)
    UAPP_GETTER(uint32_t, getTriggeringItemId, triggeringItemId)
    UAPP_GETTER_SPAN(uint32_t, getLinksToAdd, linksToAdd, linksToAddSize)
    UAPP_GETTER_SPAN(uint32_t, getLinksToRemove, linksToRemove, linksToRemoveSize)
};

/**
 * UA_SetTriggeringResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.5
 */
class SetTriggeringResponse
    : public TypeWrapper<UA_SetTriggeringResponse, UA_TYPES_SETTRIGGERINGRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, getAddResults, addResults, addResultsSize);
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getAddDiagnosticInfos, addDiagnosticInfos, addDiagnosticInfosSize
    )
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, getRemoveResults, removeResults, removeResultsSize);
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getRemoveDiagnosticInfos, removeDiagnosticInfos, removeDiagnosticInfosSize
    )
};

/**
 * UA_DeleteMonitoredItemsRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.6
 */
class DeleteMonitoredItemsRequest
    : public TypeWrapper<UA_DeleteMonitoredItemsRequest, UA_TYPES_DELETEMONITOREDITEMSREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    DeleteMonitoredItemsRequest(
        RequestHeader requestHeader, uint32_t subscriptionId, Span<const uint32_t> monitoredItemIds
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->subscriptionId = subscriptionId;
        handle()->monitoredItemIdsSize = monitoredItemIds.size();
        handle()->monitoredItemIds = detail::toNativeArray(monitoredItemIds);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER(uint32_t, getSubscriptionId, subscriptionId)
    UAPP_GETTER_SPAN(uint32_t, getMonitoredItemIds, monitoredItemIds, monitoredItemIdsSize)
};

/**
 * UA_DeleteMonitoredItemsResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.6
 */
class DeleteMonitoredItemsResponse
    : public TypeWrapper<UA_DeleteMonitoredItemsResponse, UA_TYPES_DELETEMONITOREDITEMSRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, getResults, results, resultsSize);
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_CreateSubscriptionRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.2
 */
class CreateSubscriptionRequest
    : public TypeWrapper<UA_CreateSubscriptionRequest, UA_TYPES_CREATESUBSCRIPTIONREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    CreateSubscriptionRequest(
        RequestHeader requestHeader,
        double requestedPublishingInterval,
        uint32_t requestedLifetimeCount,
        uint32_t requestedMaxKeepAliveCount,
        uint32_t maxNotificationsPerPublish,
        bool publishingEnabled,
        uint8_t priority
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->requestedPublishingInterval = requestedPublishingInterval;
        handle()->requestedLifetimeCount = requestedLifetimeCount;
        handle()->requestedMaxKeepAliveCount = requestedMaxKeepAliveCount;
        handle()->maxNotificationsPerPublish = maxNotificationsPerPublish;
        handle()->publishingEnabled = publishingEnabled;
        handle()->priority = priority;
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER(double, getRequestedPublishingInterval, requestedPublishingInterval)
    UAPP_GETTER(uint32_t, getRequestedLifetimeCount, requestedLifetimeCount)
    UAPP_GETTER(uint32_t, getRequestedMaxKeepAliveCount, requestedMaxKeepAliveCount)
    UAPP_GETTER(uint32_t, getMaxNotificationsPerPublish, maxNotificationsPerPublish)
    UAPP_GETTER(bool, getPublishingEnabled, publishingEnabled)
    UAPP_GETTER(uint8_t, getPriority, priority)
};

/**
 * UA_CreateSubscriptionResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.2
 */
class CreateSubscriptionResponse
    : public TypeWrapper<UA_CreateSubscriptionResponse, UA_TYPES_CREATESUBSCRIPTIONRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER(uint32_t, getSubscriptionId, subscriptionId)
    UAPP_GETTER(bool, getRevisedPublishingInterval, revisedPublishingInterval)
    UAPP_GETTER(uint32_t, getRevisedLifetimeCount, revisedLifetimeCount)
    UAPP_GETTER(uint32_t, getRevisedMaxKeepAliveCount, revisedMaxKeepAliveCount)
};

/**
 * UA_ModifySubscriptionRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.3
 */
class ModifySubscriptionRequest
    : public TypeWrapper<UA_ModifySubscriptionRequest, UA_TYPES_MODIFYSUBSCRIPTIONREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    ModifySubscriptionRequest(
        RequestHeader requestHeader,
        uint32_t subscriptionId,
        double requestedPublishingInterval,
        uint32_t requestedLifetimeCount,
        uint32_t requestedMaxKeepAliveCount,
        uint32_t maxNotificationsPerPublish,
        uint8_t priority
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->subscriptionId = subscriptionId;
        handle()->requestedPublishingInterval = requestedPublishingInterval;
        handle()->requestedLifetimeCount = requestedLifetimeCount;
        handle()->requestedMaxKeepAliveCount = requestedMaxKeepAliveCount;
        handle()->maxNotificationsPerPublish = maxNotificationsPerPublish;
        handle()->priority = priority;
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER(uint32_t, getSubscriptionId, subscriptionId)
    UAPP_GETTER(double, getRequestedPublishingInterval, requestedPublishingInterval)
    UAPP_GETTER(uint32_t, getRequestedLifetimeCount, requestedLifetimeCount)
    UAPP_GETTER(uint32_t, getRequestedMaxKeepAliveCount, requestedMaxKeepAliveCount)
    UAPP_GETTER(uint32_t, getMaxNotificationsPerPublish, maxNotificationsPerPublish)
    UAPP_GETTER(uint8_t, getPriority, priority)
};

/**
 * UA_ModifySubscriptionResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.3
 */
class ModifySubscriptionResponse
    : public TypeWrapper<UA_ModifySubscriptionResponse, UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER(bool, getRevisedPublishingInterval, revisedPublishingInterval)
    UAPP_GETTER(uint32_t, getRevisedLifetimeCount, revisedLifetimeCount)
    UAPP_GETTER(uint32_t, getRevisedMaxKeepAliveCount, revisedMaxKeepAliveCount)
};

/**
 * UA_SetPublishingModeRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.4
 */
class SetPublishingModeRequest
    : public TypeWrapper<UA_SetPublishingModeRequest, UA_TYPES_SETPUBLISHINGMODEREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    SetPublishingModeRequest(
        RequestHeader requestHeader, bool publishingEnabled, Span<const uint32_t> subscriptionIds
    ) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->publishingEnabled = publishingEnabled;
        handle()->subscriptionIdsSize = subscriptionIds.size();
        handle()->subscriptionIds = detail::toNativeArray(subscriptionIds);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER(bool, getPublishingEnabled, publishingEnabled)
    UAPP_GETTER_SPAN(uint32_t, getSubscriptionIds, subscriptionIds, subscriptionIdsSize)
};

/**
 * UA_SetPublishingModeResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.4
 */
class SetPublishingModeResponse
    : public TypeWrapper<UA_SetPublishingModeResponse, UA_TYPES_SETPUBLISHINGMODERESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, getResults, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_StatusChangeNotification wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.25.4
 */
class StatusChangeNotification
    : public TypeWrapper<UA_StatusChangeNotification, UA_TYPES_STATUSCHANGENOTIFICATION> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(StatusCode, getStatus, status)
    UAPP_GETTER_WRAPPER(DiagnosticInfo, getDiagnosticInfo, diagnosticInfo)
};

/**
 * UA_DeleteSubscriptionsRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.8
 */
class DeleteSubscriptionsRequest
    : public TypeWrapper<UA_DeleteSubscriptionsRequest, UA_TYPES_DELETESUBSCRIPTIONSREQUEST> {
public:
    using TypeWrapper::TypeWrapper;

    DeleteSubscriptionsRequest(RequestHeader requestHeader, Span<const uint32_t> subscriptionIds) {
        handle()->requestHeader = detail::toNative(std::move(requestHeader));
        handle()->subscriptionIdsSize = subscriptionIds.size();
        handle()->subscriptionIds = detail::toNativeArray(subscriptionIds);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_GETTER_SPAN(uint32_t, getSubscriptionIds, subscriptionIds, subscriptionIdsSize)
};

/**
 * UA_DeleteSubscriptionsResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.8
 */
class DeleteSubscriptionsResponse
    : public TypeWrapper<UA_DeleteSubscriptionsResponse, UA_TYPES_DELETESUBSCRIPTIONSRESPONSE> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, getResults, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

#endif

/* ----------------------------------------- Historizing ---------------------------------------- */

/**
 * Perform update type for structured data history updates.
 * @see UA_PerformUpdateType
 * @see https://reference.opcfoundation.org/Core/Part11/v104/docs/6.8.3
 */
enum class PerformUpdateType : int32_t {
    // clang-format off
    Insert  = 1,
    Replace = 2,
    Update  = 3,
    Remove  = 4,
    // clang-format on
};

/* -------------------------------------- Type description -------------------------------------- */

#ifdef UA_ENABLE_TYPEDESCRIPTION

/**
 * Structure type.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.49
 */
enum class StructureType : int32_t {
    // clang-format off
    Structure                   = 0,
    StructureWithOptionalFields = 1,
    Union                       = 2,
    // clang-format on
};

/**
 * UA_StructureField wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.51
 */
class StructureField : public TypeWrapper<UA_StructureField, UA_TYPES_STRUCTUREFIELD> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(String, getName, name)
    UAPP_GETTER_WRAPPER(LocalizedText, getDescription, description)
    UAPP_GETTER_WRAPPER(NodeId, getDataType, dataType)
    UAPP_GETTER_CAST(ValueRank, getValueRank, valueRank)
    UAPP_GETTER_SPAN(uint32_t, getArrayDimensions, arrayDimensions, arrayDimensionsSize)
    UAPP_GETTER(uint32_t, getMaxStringLength, maxStringLength)
    UAPP_GETTER(bool, getIsOptional, isOptional)
};

/**
 * UA_StructureDefinition wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.48
 */
class StructureDefinition
    : public TypeWrapper<UA_StructureDefinition, UA_TYPES_STRUCTUREDEFINITION> {
public:
    using TypeWrapper::TypeWrapper;

    UAPP_GETTER_WRAPPER(NodeId, getDefaultEncodingId, defaultEncodingId)
    UAPP_GETTER_WRAPPER(NodeId, getBaseDataType, baseDataType)
    UAPP_GETTER_CAST(StructureType, getStructureType, structureType)
    UAPP_GETTER_SPAN_WRAPPER(StructureField, getFields, fields, fieldsSize)
};

/**
 * UA_EnumField wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.52
 */
class EnumField : public TypeWrapper<UA_EnumField, UA_TYPES_ENUMFIELD> {
public:
    using TypeWrapper::TypeWrapper;

    EnumField(int64_t value, std::string_view name)
        : EnumField(value, {"", name}, {}, name) {}

    EnumField(
        int64_t value, LocalizedText displayName, LocalizedText description, std::string_view name
    ) {
        handle()->value = value;
        handle()->displayName = detail::toNative(std::move(displayName));
        handle()->description = detail::toNative(std::move(description));
        handle()->name = detail::toNative(name);
    }

    UAPP_GETTER(int64_t, getValue, value)
    UAPP_GETTER_WRAPPER(LocalizedText, getDisplayName, displayName)
    UAPP_GETTER_WRAPPER(LocalizedText, getDescription, description)
    UAPP_GETTER_WRAPPER(String, getName, name)
};

/**
 * UA_EnumDefinition wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.50
 */
class EnumDefinition : public TypeWrapper<UA_EnumDefinition, UA_TYPES_ENUMDEFINITION> {
public:
    using TypeWrapper::TypeWrapper;

    EnumDefinition(std::initializer_list<EnumField> fields)
        : EnumDefinition({fields.begin(), fields.size()}) {}

    explicit EnumDefinition(Span<const EnumField> fields) {
        handle()->fieldsSize = fields.size();
        handle()->fields = detail::toNativeArray(fields);
    }

    UAPP_GETTER_SPAN_WRAPPER(EnumField, getFields, fields, fieldsSize)
};

#endif

/**
 * @}
 */

}  // namespace opcua
