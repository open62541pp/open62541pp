#pragma once

#include <cstdint>
#include <initializer_list>
#include <string_view>
#include <type_traits>
#include <utility>  // forward, move
#include <variant>

#include "open62541pp/bitmask.hpp"
#include "open62541pp/common.hpp"  // AttributeId, ...
#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/detail/traits.hpp"  // IsOneOf
#include "open62541pp/detail/types_conversion.hpp"  // makeNative, makeNativeArray
#include "open62541pp/detail/types_handling.hpp"  // deallocateArray, copyArray
#include "open62541pp/span.hpp"
#include "open62541pp/typeregistry.hpp"  // getDataType
#include "open62541pp/types.hpp"
#include "open62541pp/ua/nodeids.hpp"  // ReferenceTypeId
#include "open62541pp/ua/typeregistry.hpp"
#include "open62541pp/wrapper.hpp"

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

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define UAPP_GETTER(Type, member)                                                                  \
    Type member() const noexcept {                                                                 \
        return handle()->member;                                                                   \
    }

#define UAPP_GETTER_CAST(Type, member)                                                             \
    Type member() const noexcept {                                                                 \
        return static_cast<Type>(handle()->member);                                                \
    }

#define UAPP_GETTER_WRAPPER_CONST(Type, member)                                                    \
    const Type& member() const noexcept {                                                          \
        return asWrapper<Type>(handle()->member);                                                  \
    }
#define UAPP_GETTER_WRAPPER_NONCONST(Type, member)                                                 \
    Type& member() noexcept {                                                                      \
        return asWrapper<Type>(handle()->member);                                                  \
    }
#define UAPP_GETTER_WRAPPER(Type, member)                                                          \
    UAPP_GETTER_WRAPPER_CONST(Type, member)                                                        \
    UAPP_GETTER_WRAPPER_NONCONST(Type, member)

#define UAPP_GETTER_SPAN(Type, memberArray, memberSize)                                            \
    Span<const Type> memberArray() const noexcept {                                                \
        return {handle()->memberArray, handle()->memberSize};                                      \
    }                                                                                              \
    Span<Type> memberArray() noexcept {                                                            \
        return {handle()->memberArray, handle()->memberSize};                                      \
    }

#define UAPP_GETTER_SPAN_WRAPPER(Type, memberArray, memberSize)                                    \
    Span<const Type> memberArray() const noexcept {                                                \
        return {asWrapper<Type>(handle()->memberArray), handle()->memberSize};                     \
    }                                                                                              \
    Span<Type> memberArray() noexcept {                                                            \
        return {asWrapper<Type>(handle()->memberArray), handle()->memberSize};                     \
    }

// NOLINTEND(cppcoreguidelines-macro-usage)

namespace opcua {
inline namespace ua {

/// IntegerId.
/// @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.19
using IntegerId = uint32_t;

/**
 * @addtogroup Wrapper
 * @{
 */

/**
 * UA_EnumValueType wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.39
 */
class EnumValueType : public WrapperNative<UA_EnumValueType, UA_TYPES_ENUMVALUETYPE> {
public:
    using Wrapper::Wrapper;

    EnumValueType(int64_t value, LocalizedText displayName, LocalizedText description) {
        handle()->value = value;
        handle()->displayName = detail::makeNative(std::move(displayName));
        handle()->description = detail::makeNative(std::move(description));
    }

    UAPP_GETTER(int64_t, value)
    UAPP_GETTER_WRAPPER(LocalizedText, displayName)
    UAPP_GETTER_WRAPPER(LocalizedText, description)
};

/**
 * Application type.
 * @see UA_ApplicationType
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.4
 */
enum class ApplicationType : int32_t {
    Server = 0,
    Client = 1,
    ClientAndServer = 2,
    DiscoveryServer = 3,
};

/**
 * UA_ApplicationDescription wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.2
 */
class ApplicationDescription
    : public WrapperNative<UA_ApplicationDescription, UA_TYPES_APPLICATIONDESCRIPTION> {
public:
    using Wrapper::Wrapper;

    ApplicationDescription(
        std::string_view applicationUri,
        std::string_view productUri,
        LocalizedText applicationName,
        ApplicationType applicationType,
        std::string_view gatewayServerUri,
        std::string_view discoveryProfileUri,
        Span<const String> discoveryUrls
    ) {
        handle()->applicationUri = detail::makeNative(applicationUri);
        handle()->productUri = detail::makeNative(productUri);
        handle()->applicationName = detail::makeNative(std::move(applicationName));
        handle()->applicationType = static_cast<UA_ApplicationType>(applicationType);
        handle()->gatewayServerUri = detail::makeNative(gatewayServerUri);
        handle()->discoveryProfileUri = detail::makeNative(discoveryProfileUri);
        handle()->discoveryUrlsSize = discoveryUrls.size();
        handle()->discoveryUrls = detail::makeNativeArray(discoveryUrls);
    }

    UAPP_GETTER_WRAPPER(String, applicationUri)
    UAPP_GETTER_WRAPPER(String, productUri)
    UAPP_GETTER_WRAPPER(LocalizedText, applicationName)
    UAPP_GETTER_CAST(ApplicationType, applicationType)
    UAPP_GETTER_WRAPPER(String, gatewayServerUri)
    UAPP_GETTER_WRAPPER(String, discoveryProfileUri)
    UAPP_GETTER_SPAN_WRAPPER(String, discoveryUrls, discoveryUrlsSize)
};

/**
 * UA_RequestHeader wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.33
 */
class RequestHeader : public WrapperNative<UA_RequestHeader, UA_TYPES_REQUESTHEADER> {
public:
    using Wrapper::Wrapper;

    RequestHeader(
        NodeId authenticationToken,
        DateTime timestamp,
        IntegerId requestHandle,
        uint32_t returnDiagnostics,
        std::string_view auditEntryId,
        uint32_t timeoutHint,
        ExtensionObject additionalHeader
    ) {
        handle()->authenticationToken = detail::makeNative(std::move(authenticationToken));
        handle()->timestamp = timestamp;
        handle()->requestHandle = requestHandle;
        handle()->returnDiagnostics = returnDiagnostics;
        handle()->auditEntryId = detail::makeNative(auditEntryId);
        handle()->timeoutHint = timeoutHint;
        handle()->additionalHeader = detail::makeNative(std::move(additionalHeader));
    }

    UAPP_GETTER_WRAPPER(NodeId, authenticationToken)
    UAPP_GETTER_WRAPPER(DateTime, timestamp)
    UAPP_GETTER(IntegerId, requestHandle)
    UAPP_GETTER(uint32_t, returnDiagnostics)
    UAPP_GETTER_WRAPPER(String, auditEntryId)
    UAPP_GETTER(uint32_t, timeoutHint)
    UAPP_GETTER_WRAPPER(ExtensionObject, additionalHeader)
};

/**
 * UA_ResponseHeader wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.34
 */
class ResponseHeader : public WrapperNative<UA_ResponseHeader, UA_TYPES_RESPONSEHEADER> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(DateTime, timestamp)
    UAPP_GETTER(IntegerId, requestHandle)
    UAPP_GETTER(StatusCode, serviceResult)
    UAPP_GETTER_WRAPPER(DiagnosticInfo, serviceDiagnostics)
    UAPP_GETTER_SPAN_WRAPPER(String, stringTable, stringTableSize)
    UAPP_GETTER_WRAPPER(ExtensionObject, additionalHeader)
};

/**
 * Message security mode.
 * @see UA_MessageSecurityMode
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.20
 */
enum class MessageSecurityMode : int32_t {
    // clang-format off
    Invalid        = 0,  ///< Will always be rejected
    None           = 1,  ///< No security applied
    Sign           = 2,  ///< All messages are signed but not encrypted
    SignAndEncrypt = 3,  ///< All messages are signed and encrypted
    // clang-format on
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
class UserTokenPolicy : public WrapperNative<UA_UserTokenPolicy, UA_TYPES_USERTOKENPOLICY> {
public:
    using Wrapper::Wrapper;

    UserTokenPolicy(
        std::string_view policyId,
        UserTokenType tokenType,
        std::string_view issuedTokenType,
        std::string_view issuerEndpointUrl,
        std::string_view securityPolicyUri
    ) {
        handle()->policyId = detail::makeNative(policyId);
        handle()->tokenType = static_cast<UA_UserTokenType>(tokenType);
        handle()->issuedTokenType = detail::makeNative(issuedTokenType);
        handle()->issuerEndpointUrl = detail::makeNative(issuerEndpointUrl);
        handle()->securityPolicyUri = detail::makeNative(securityPolicyUri);
    }

    UAPP_GETTER_WRAPPER(String, policyId)
    UAPP_GETTER_CAST(UserTokenType, tokenType)
    UAPP_GETTER_WRAPPER(String, issuedTokenType)
    UAPP_GETTER_WRAPPER(String, issuerEndpointUrl)
    UAPP_GETTER_WRAPPER(String, securityPolicyUri)
};

/**
 * UA_EndpointDescription wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.14
 */
class EndpointDescription
    : public WrapperNative<UA_EndpointDescription, UA_TYPES_ENDPOINTDESCRIPTION> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(String, endpointUrl)
    UAPP_GETTER_WRAPPER(ApplicationDescription, server)
    UAPP_GETTER_WRAPPER(ByteString, serverCertificate)
    UAPP_GETTER_CAST(MessageSecurityMode, securityMode)
    UAPP_GETTER_WRAPPER(String, securityPolicyUri)
    UAPP_GETTER_SPAN_WRAPPER(UserTokenPolicy, userIdentityTokens, userIdentityTokensSize)
    UAPP_GETTER_WRAPPER(String, transportProfileUri)
    UAPP_GETTER(UA_Byte, securityLevel)
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

constexpr std::true_type isBitmaskEnum(NodeAttributesMask);

// Specifialized macros to generate getters/setters for `UA_*Attribute` classes.
// The `specifiedAttributes` mask is automatically updated in the setter methods.
// A fluent interface is used for the setter methods.

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define UAPP_NODEATTR(Type, suffix, member, flag)                                                  \
    UAPP_GETTER(Type, member)                                                                      \
    auto& set##suffix(Type member) noexcept {                                                      \
        handle()->specifiedAttributes |= flag;                                                     \
        handle()->member = member;                                                                 \
        return *this;                                                                              \
    }
#define UAPP_NODEATTR_CAST(Type, suffix, member, flag)                                             \
    UAPP_GETTER_CAST(Type, member)                                                                 \
    auto& set##suffix(Type member) noexcept {                                                      \
        handle()->specifiedAttributes |= flag;                                                     \
        handle()->member = static_cast<decltype(handle()->member)>(member);                        \
        return *this;                                                                              \
    }
#define UAPP_NODEATTR_WRAPPER(Type, suffix, member, flag)                                          \
    UAPP_GETTER_WRAPPER_CONST(Type, member)                                                        \
    auto& set##suffix(Type member) {                                                               \
        handle()->specifiedAttributes |= flag;                                                     \
        asWrapper<Type>(handle()->member) = std::move(member);                                     \
        return *this;                                                                              \
    }
#define UAPP_NODEATTR_ARRAY(Type, suffix, member, memberSize, flag)                                \
    UAPP_GETTER_SPAN(Type, member, memberSize)                                                     \
    auto& set##suffix(Span<const Type> member) {                                                   \
        const auto& type = opcua::getDataType<Type>();                                             \
        handle()->specifiedAttributes |= flag;                                                     \
        detail::deallocateArray(handle()->member, handle()->memberSize, type);                     \
        handle()->member = detail::copyArray(member.data(), member.size(), type);                  \
        handle()->memberSize = member.size();                                                      \
        return *this;                                                                              \
    }
#define UAPP_NODEATTR_COMMON                                                                       \
    UAPP_GETTER(Bitmask<NodeAttributesMask>, specifiedAttributes)                                  \
    UAPP_NODEATTR_WRAPPER(                                                                         \
        LocalizedText, DisplayName, displayName, UA_NODEATTRIBUTESMASK_DISPLAYNAME                 \
    )                                                                                              \
    UAPP_NODEATTR_WRAPPER(                                                                         \
        LocalizedText, Description, description, UA_NODEATTRIBUTESMASK_DESCRIPTION                 \
    )                                                                                              \
    UAPP_NODEATTR_CAST(Bitmask<WriteMask>, WriteMask, writeMask, UA_NODEATTRIBUTESMASK_WRITEMASK)  \
    UAPP_NODEATTR_CAST(                                                                            \
        Bitmask<WriteMask>, UserWriteMask, userWriteMask, UA_NODEATTRIBUTESMASK_USERWRITEMASK      \
    )

// NOLINTEND(cppcoreguidelines-macro-usage)

/**
 * UA_NodeAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24
 */
class NodeAttributes : public WrapperNative<UA_NodeAttributes, UA_TYPES_NODEATTRIBUTES> {
public:
    using Wrapper::Wrapper;

    UAPP_NODEATTR_COMMON
};

/**
 * UA_ObjectAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.2
 */
class ObjectAttributes : public WrapperNative<UA_ObjectAttributes, UA_TYPES_OBJECTATTRIBUTES> {
public:
    using Wrapper::Wrapper;

    /// Construct with default attribute definitions.
    ObjectAttributes()
        : ObjectAttributes{UA_ObjectAttributes_default} {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR_CAST(
        Bitmask<EventNotifier>, EventNotifier, eventNotifier, UA_NODEATTRIBUTESMASK_EVENTNOTIFIER
    )
};

/**
 * UA_VariableAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.3
 */
class VariableAttributes
    : public WrapperNative<UA_VariableAttributes, UA_TYPES_VARIABLEATTRIBUTES> {
public:
    using Wrapper::Wrapper;

    /// Construct with default attribute definitions.
    VariableAttributes()
        : VariableAttributes{UA_VariableAttributes_default} {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR_WRAPPER(Variant, Value, value, UA_NODEATTRIBUTESMASK_VALUE)

    /// @deprecated Use setValue(Variant{...}) instead
    template <typename... Args>
    [[deprecated("use setValue(Variant{...}) instead")]]
    auto& setValueScalar(Args&&... args) {
        return setValue(Variant{std::forward<Args>(args)...});
    }

    /// @deprecated Use setValue(Variant{...}) instead
    template <typename... Args>
    [[deprecated("use setValue(Variant{...}) instead")]]
    auto& setValueArray(Args&&... args) {
        return setValue(Variant{std::forward<Args>(args)...});
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
    UAPP_NODEATTR_CAST(
        Bitmask<AccessLevel>, AccessLevel, accessLevel, UA_NODEATTRIBUTESMASK_ACCESSLEVEL
    )
    UAPP_NODEATTR_CAST(
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
class MethodAttributes : public WrapperNative<UA_MethodAttributes, UA_TYPES_METHODATTRIBUTES> {
public:
    using Wrapper::Wrapper;

    /// Construct with default attribute definitions.
    MethodAttributes()
        : MethodAttributes{UA_MethodAttributes_default} {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, Executable, executable, UA_NODEATTRIBUTESMASK_EXECUTABLE)
    UAPP_NODEATTR(bool, UserExecutable, userExecutable, UA_NODEATTRIBUTESMASK_USEREXECUTABLE)
};

/**
 * UA_ObjectTypeAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.5
 */
class ObjectTypeAttributes
    : public WrapperNative<UA_ObjectTypeAttributes, UA_TYPES_OBJECTTYPEATTRIBUTES> {
public:
    using Wrapper::Wrapper;

    /// Construct with default attribute definitions.
    ObjectTypeAttributes()
        : ObjectTypeAttributes{UA_ObjectTypeAttributes_default} {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, IsAbstract, isAbstract, UA_NODEATTRIBUTESMASK_ISABSTRACT)
};

/**
 * UA_VariableAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.6
 */
class VariableTypeAttributes
    : public WrapperNative<UA_VariableTypeAttributes, UA_TYPES_VARIABLETYPEATTRIBUTES> {
public:
    using Wrapper::Wrapper;

    /// Construct with default attribute definitions.
    VariableTypeAttributes()
        : VariableTypeAttributes{UA_VariableTypeAttributes_default} {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR_WRAPPER(Variant, Value, value, UA_NODEATTRIBUTESMASK_VALUE)

    /// @deprecated Use setValue(Variant{...}) instead
    template <typename... Args>
    [[deprecated("use setValue(Variant{...}) instead")]]
    auto& setValueScalar(Args&&... args) {
        return setValue(Variant{std::forward<Args>(args)...});
    }

    /// @deprecated Use setValue(Variant{...}) instead
    template <typename... Args>
    [[deprecated("use setValue(Variant{...}) instead")]]
    auto& setValueArray(Args&&... args) {
        return setValue(Variant{std::forward<Args>(args)...});
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
    : public WrapperNative<UA_ReferenceTypeAttributes, UA_TYPES_REFERENCETYPEATTRIBUTES> {
public:
    using Wrapper::Wrapper;

    /// Construct with default attribute definitions.
    ReferenceTypeAttributes()
        : ReferenceTypeAttributes{UA_ReferenceTypeAttributes_default} {}

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
class DataTypeAttributes
    : public WrapperNative<UA_DataTypeAttributes, UA_TYPES_DATATYPEATTRIBUTES> {
public:
    using Wrapper::Wrapper;

    /// Construct with default attribute definitions.
    DataTypeAttributes()
        : DataTypeAttributes{UA_DataTypeAttributes_default} {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, IsAbstract, isAbstract, UA_NODEATTRIBUTESMASK_ISABSTRACT)
};

/**
 * UA_ViewAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.9
 */
class ViewAttributes : public WrapperNative<UA_ViewAttributes, UA_TYPES_VIEWATTRIBUTES> {
public:
    using Wrapper::Wrapper;

    /// Construct with default attribute definitions.
    ViewAttributes()
        : ViewAttributes{UA_ViewAttributes_default} {}

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, IsAbstract, containsNoLoops, UA_NODEATTRIBUTESMASK_CONTAINSNOLOOPS)
    UAPP_NODEATTR_CAST(
        Bitmask<EventNotifier>, EventNotifier, eventNotifier, UA_NODEATTRIBUTESMASK_EVENTNOTIFIER
    )
};

#undef UAPP_NODEATTR
#undef UAPP_NODEATTR_CAST
#undef UAPP_NODEATTR_WRAPPER
#undef UAPP_NODEATTR_ARRAY
#undef UAPP_NODEATTR_COMMON

/* ---------------------------------------------------------------------------------------------- */

/**
 * UA_UserIdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41
 */
class UserIdentityToken : public WrapperNative<UA_UserIdentityToken, UA_TYPES_USERIDENTITYTOKEN> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(String, policyId)
};

/**
 * UA_AnonymousIdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41.3
 */
class AnonymousIdentityToken
    : public WrapperNative<UA_AnonymousIdentityToken, UA_TYPES_ANONYMOUSIDENTITYTOKEN> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(String, policyId)
};

/**
 * UA_UserNameIdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41.4
 */
class UserNameIdentityToken
    : public WrapperNative<UA_UserNameIdentityToken, UA_TYPES_USERNAMEIDENTITYTOKEN> {
public:
    using Wrapper::Wrapper;

    UserNameIdentityToken(
        std::string_view userName,
        std::string_view password,
        std::string_view encryptionAlgorithm = {}
    ) {
        handle()->userName = detail::makeNative(userName);
        handle()->password = detail::makeNative(password);
        handle()->encryptionAlgorithm = detail::makeNative(encryptionAlgorithm);
    }

    UAPP_GETTER_WRAPPER(String, policyId)
    UAPP_GETTER_WRAPPER(String, userName)
    UAPP_GETTER_WRAPPER(ByteString, password)
    UAPP_GETTER_WRAPPER(String, encryptionAlgorithm)
};

/**
 * UA_X509IdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41.5
 */
class X509IdentityToken : public WrapperNative<UA_X509IdentityToken, UA_TYPES_X509IDENTITYTOKEN> {
public:
    using Wrapper::Wrapper;

    explicit X509IdentityToken(ByteString certificateData) {
        handle()->certificateData = detail::makeNative(std::move(certificateData));
    }

    UAPP_GETTER_WRAPPER(String, policyId)
    UAPP_GETTER_WRAPPER(ByteString, certificateData)
};

/**
 * UA_IssuedIdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41.6
 */
class IssuedIdentityToken
    : public WrapperNative<UA_IssuedIdentityToken, UA_TYPES_ISSUEDIDENTITYTOKEN> {
public:
    using Wrapper::Wrapper;

    explicit IssuedIdentityToken(ByteString tokenData, std::string_view encryptionAlgorithm = {}) {
        handle()->tokenData = detail::makeNative(std::move(tokenData));
        handle()->encryptionAlgorithm = detail::makeNative(encryptionAlgorithm);
    }

    UAPP_GETTER_WRAPPER(String, policyId)
    UAPP_GETTER_WRAPPER(ByteString, tokenData)
    UAPP_GETTER_WRAPPER(String, encryptionAlgorithm)
};

/**
 * UA_AddNodesItem wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.2
 */
class AddNodesItem : public WrapperNative<UA_AddNodesItem, UA_TYPES_ADDNODESITEM> {
public:
    using Wrapper::Wrapper;

    AddNodesItem(
        ExpandedNodeId parentNodeId,
        NodeId referenceTypeId,
        ExpandedNodeId requestedNewNodeId,
        QualifiedName browseName,
        NodeClass nodeClass,
        ExtensionObject nodeAttributes,
        ExpandedNodeId typeDefinition
    ) {
        handle()->parentNodeId = detail::makeNative(std::move(parentNodeId));
        handle()->referenceTypeId = detail::makeNative(std::move(referenceTypeId));
        handle()->requestedNewNodeId = detail::makeNative(std::move(requestedNewNodeId));
        handle()->browseName = detail::makeNative(std::move(browseName));
        handle()->nodeClass = static_cast<UA_NodeClass>(nodeClass);
        handle()->nodeAttributes = detail::makeNative(std::move(nodeAttributes));
        handle()->typeDefinition = detail::makeNative(std::move(typeDefinition));
    }

    UAPP_GETTER_WRAPPER(ExpandedNodeId, parentNodeId)
    UAPP_GETTER_WRAPPER(NodeId, referenceTypeId)
    UAPP_GETTER_WRAPPER(ExpandedNodeId, requestedNewNodeId)
    UAPP_GETTER_WRAPPER(QualifiedName, browseName)
    UAPP_GETTER_CAST(NodeClass, nodeClass)
    UAPP_GETTER_WRAPPER(ExtensionObject, nodeAttributes)
    UAPP_GETTER_WRAPPER(ExpandedNodeId, typeDefinition)
};

/**
 * UA_AddNodesResult wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.2
 */
class AddNodesResult : public WrapperNative<UA_AddNodesResult, UA_TYPES_ADDNODESRESULT> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER(StatusCode, statusCode)
    UAPP_GETTER_WRAPPER(NodeId, addedNodeId)
};

/**
 * UA_AddNodesRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.2
 */
class AddNodesRequest : public WrapperNative<UA_AddNodesRequest, UA_TYPES_ADDNODESREQUEST> {
public:
    using Wrapper::Wrapper;

    AddNodesRequest(RequestHeader requestHeader, Span<const AddNodesItem> nodesToAdd) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->nodesToAddSize = nodesToAdd.size();
        handle()->nodesToAdd = detail::makeNativeArray(nodesToAdd);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(AddNodesItem, nodesToAdd, nodesToAddSize)
};

/**
 * UA_AddNodesResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.2
 */
class AddNodesResponse : public WrapperNative<UA_AddNodesResponse, UA_TYPES_ADDNODESRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(AddNodesResult, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_AddReferencesItem wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.3
 */
class AddReferencesItem : public WrapperNative<UA_AddReferencesItem, UA_TYPES_ADDREFERENCESITEM> {
public:
    using Wrapper::Wrapper;

    AddReferencesItem(
        NodeId sourceNodeId,
        NodeId referenceTypeId,
        bool isForward,
        std::string_view targetServerUri,
        ExpandedNodeId targetNodeId,
        NodeClass targetNodeClass
    ) {
        handle()->sourceNodeId = detail::makeNative(std::move(sourceNodeId));
        handle()->referenceTypeId = detail::makeNative(std::move(referenceTypeId));
        handle()->isForward = isForward;
        handle()->targetServerUri = detail::makeNative(targetServerUri);
        handle()->targetNodeId = detail::makeNative(std::move(targetNodeId));
        handle()->targetNodeClass = static_cast<UA_NodeClass>(targetNodeClass);
    }

    UAPP_GETTER_WRAPPER(NodeId, sourceNodeId)
    UAPP_GETTER_WRAPPER(NodeId, referenceTypeId)
    UAPP_GETTER(bool, isForward)
    UAPP_GETTER_WRAPPER(String, targetServerUri)
    UAPP_GETTER_WRAPPER(ExpandedNodeId, targetNodeId)
    UAPP_GETTER_CAST(NodeClass, targetNodeClass)
};

/**
 * UA_AddReferencesRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.3
 */
class AddReferencesRequest
    : public WrapperNative<UA_AddReferencesRequest, UA_TYPES_ADDREFERENCESREQUEST> {
public:
    using Wrapper::Wrapper;

    AddReferencesRequest(
        RequestHeader requestHeader, Span<const AddReferencesItem> referencesToAdd
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->referencesToAddSize = referencesToAdd.size();
        handle()->referencesToAdd = detail::makeNativeArray(referencesToAdd);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(AddReferencesItem, referencesToAdd, referencesToAddSize)
};

/**
 * UA_AddReferencesResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.3
 */
class AddReferencesResponse
    : public WrapperNative<UA_AddReferencesResponse, UA_TYPES_ADDREFERENCESRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_DeleteNodesItem wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.4
 */
class DeleteNodesItem : public WrapperNative<UA_DeleteNodesItem, UA_TYPES_DELETENODESITEM> {
public:
    using Wrapper::Wrapper;

    DeleteNodesItem(NodeId nodeId, bool deleteTargetReferences) {
        handle()->nodeId = detail::makeNative(std::move(nodeId));
        handle()->deleteTargetReferences = deleteTargetReferences;
    }

    UAPP_GETTER_WRAPPER(NodeId, nodeId)
    UAPP_GETTER(bool, deleteTargetReferences)
};

/**
 * UA_DeleteNodesRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.4
 */
class DeleteNodesRequest
    : public WrapperNative<UA_DeleteNodesRequest, UA_TYPES_DELETENODESREQUEST> {
public:
    using Wrapper::Wrapper;

    DeleteNodesRequest(RequestHeader requestHeader, Span<const DeleteNodesItem> nodesToDelete) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->nodesToDeleteSize = nodesToDelete.size();
        handle()->nodesToDelete = detail::makeNativeArray(nodesToDelete);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(DeleteNodesItem, nodesToDelete, nodesToDeleteSize)
};

/**
 * UA_DeleteNodesResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.4
 */
class DeleteNodesResponse
    : public WrapperNative<UA_DeleteNodesResponse, UA_TYPES_DELETENODESRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_DeleteReferencesItem wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.5
 */
class DeleteReferencesItem
    : public WrapperNative<UA_DeleteReferencesItem, UA_TYPES_DELETEREFERENCESITEM> {
public:
    using Wrapper::Wrapper;

    DeleteReferencesItem(
        NodeId sourceNodeId,
        NodeId referenceTypeId,
        bool isForward,
        ExpandedNodeId targetNodeId,
        bool deleteBidirectional
    ) {
        handle()->sourceNodeId = detail::makeNative(std::move(sourceNodeId));
        handle()->referenceTypeId = detail::makeNative(std::move(referenceTypeId));
        handle()->isForward = isForward;
        handle()->targetNodeId = detail::makeNative(std::move(targetNodeId));
        handle()->deleteBidirectional = deleteBidirectional;
    }

    UAPP_GETTER_WRAPPER(NodeId, sourceNodeId)
    UAPP_GETTER_WRAPPER(NodeId, referenceTypeId)
    UAPP_GETTER(bool, isForward)
    UAPP_GETTER_WRAPPER(ExpandedNodeId, targetNodeId)
    UAPP_GETTER(bool, deleteBidirectional)
};

/**
 * UA_DeleteReferencesRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.5
 */
class DeleteReferencesRequest
    : public WrapperNative<UA_DeleteReferencesRequest, UA_TYPES_DELETEREFERENCESREQUEST> {
public:
    using Wrapper::Wrapper;

    DeleteReferencesRequest(
        RequestHeader requestHeader, Span<const DeleteReferencesItem> referencesToDelete
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->referencesToDeleteSize = referencesToDelete.size();
        handle()->referencesToDelete = detail::makeNativeArray(referencesToDelete);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(DeleteReferencesItem, referencesToDelete, referencesToDeleteSize)
};

/**
 * UA_DeleteReferencesResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7.5
 */
class DeleteReferencesResponse
    : public WrapperNative<UA_DeleteReferencesResponse, UA_TYPES_DELETEREFERENCESRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_ViewDescription wrapper class.
 */
class ViewDescription : public WrapperNative<UA_ViewDescription, UA_TYPES_VIEWDESCRIPTION> {
public:
    using Wrapper::Wrapper;

    ViewDescription(NodeId viewId, DateTime timestamp, uint32_t viewVersion) {
        handle()->viewId = detail::makeNative(std::move(viewId));
        handle()->timestamp = timestamp;
        handle()->viewVersion = viewVersion;
    }

    UAPP_GETTER_WRAPPER(NodeId, viewId)
    UAPP_GETTER_WRAPPER(DateTime, timestamp)
    UAPP_GETTER(uint32_t, viewVersion)
};

/**
 * Browse direction.
 * An enumeration that specifies the direction of references to follow.
 * @see UA_BrowseDirection
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.5
 */
enum class BrowseDirection : int32_t {
    // clang-format off
    Forward = 0,
    Inverse = 1,
    Both    = 2,
    Invalid = 3,
    // clang-format on
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

constexpr std::true_type isBitmaskEnum(BrowseResultMask);

/**
 * UA_BrowseDescription wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8.2
 */
class BrowseDescription : public WrapperNative<UA_BrowseDescription, UA_TYPES_BROWSEDESCRIPTION> {
public:
    using Wrapper::Wrapper;

    BrowseDescription(
        NodeId nodeId,
        BrowseDirection browseDirection,
        NodeId referenceTypeId = ReferenceTypeId::References,
        bool includeSubtypes = true,
        Bitmask<NodeClass> nodeClassMask = NodeClass::Unspecified,
        Bitmask<BrowseResultMask> resultMask = BrowseResultMask::All
    ) {
        handle()->nodeId = detail::makeNative(std::move(nodeId));
        handle()->browseDirection = static_cast<UA_BrowseDirection>(browseDirection);
        handle()->referenceTypeId = detail::makeNative(std::move(referenceTypeId));
        handle()->includeSubtypes = includeSubtypes;
        handle()->nodeClassMask = nodeClassMask.get();
        handle()->resultMask = resultMask.get();
    }

    UAPP_GETTER_WRAPPER(NodeId, nodeId)
    UAPP_GETTER_CAST(BrowseDirection, browseDirection)
    UAPP_GETTER_WRAPPER(NodeId, referenceTypeId)
    UAPP_GETTER(bool, includeSubtypes)
    UAPP_GETTER(Bitmask<NodeClass>, nodeClassMask)
    UAPP_GETTER(Bitmask<BrowseResultMask>, resultMask)
};

/**
 * UA_BrowseRequest wrapper class.
 */
class BrowseRequest : public WrapperNative<UA_BrowseRequest, UA_TYPES_BROWSEREQUEST> {
public:
    using Wrapper::Wrapper;

    BrowseRequest(
        RequestHeader requestHeader,
        ViewDescription view,
        uint32_t requestedMaxReferencesPerNode,
        Span<const BrowseDescription> nodesToBrowse
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->view = detail::makeNative(std::move(view));
        handle()->requestedMaxReferencesPerNode = requestedMaxReferencesPerNode;
        handle()->nodesToBrowseSize = nodesToBrowse.size();
        handle()->nodesToBrowse = detail::makeNativeArray(nodesToBrowse);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER_WRAPPER(ViewDescription, view)
    UAPP_GETTER(uint32_t, requestedMaxReferencesPerNode)
    UAPP_GETTER_SPAN_WRAPPER(BrowseDescription, nodesToBrowse, nodesToBrowseSize)
};

/**
 * UA_ReferenceDescription wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.30
 */
class ReferenceDescription
    : public WrapperNative<UA_ReferenceDescription, UA_TYPES_REFERENCEDESCRIPTION> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(NodeId, referenceTypeId)
    UAPP_GETTER(bool, isForward)
    UAPP_GETTER_WRAPPER(ExpandedNodeId, nodeId)
    UAPP_GETTER_WRAPPER(QualifiedName, browseName)
    UAPP_GETTER_WRAPPER(LocalizedText, displayName)
    UAPP_GETTER_CAST(NodeClass, nodeClass)
    UAPP_GETTER_WRAPPER(ExpandedNodeId, typeDefinition)
};

/**
 * UA_BrowseResult wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.6
 */
class BrowseResult : public WrapperNative<UA_BrowseResult, UA_TYPES_BROWSERESULT> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER(StatusCode, statusCode)
    UAPP_GETTER_WRAPPER(ByteString, continuationPoint)
    UAPP_GETTER_SPAN_WRAPPER(ReferenceDescription, references, referencesSize)
};

/**
 * UA_BrowseResponse wrapper class.
 */
class BrowseResponse : public WrapperNative<UA_BrowseResponse, UA_TYPES_BROWSERESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(BrowseResult, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_BrowseNextRequest wrapper class.
 */
class BrowseNextRequest : public WrapperNative<UA_BrowseNextRequest, UA_TYPES_BROWSENEXTREQUEST> {
public:
    using Wrapper::Wrapper;

    BrowseNextRequest(
        RequestHeader requestHeader,
        bool releaseContinuationPoints,
        Span<const ByteString> continuationPoints
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->releaseContinuationPoints = releaseContinuationPoints;
        handle()->continuationPointsSize = continuationPoints.size();
        handle()->continuationPoints = detail::makeNativeArray(continuationPoints);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER(bool, releaseContinuationPoints)
    UAPP_GETTER_SPAN_WRAPPER(ByteString, continuationPoints, continuationPointsSize)
};

/**
 * UA_BrowseNextResponse wrapper class.
 */
class BrowseNextResponse
    : public WrapperNative<UA_BrowseNextResponse, UA_TYPES_BROWSENEXTRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(BrowseResult, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_RelativePathElement wrapper class.
 */
class RelativePathElement
    : public WrapperNative<UA_RelativePathElement, UA_TYPES_RELATIVEPATHELEMENT> {
public:
    using Wrapper::Wrapper;

    RelativePathElement(
        NodeId referenceTypeId, bool isInverse, bool includeSubtypes, QualifiedName targetName
    ) {
        handle()->referenceTypeId = detail::makeNative(std::move(referenceTypeId));
        handle()->isInverse = isInverse;
        handle()->includeSubtypes = includeSubtypes;
        handle()->targetName = detail::makeNative(std::move(targetName));
    }

    UAPP_GETTER_WRAPPER(NodeId, referenceTypeId)
    UAPP_GETTER(bool, isInverse)
    UAPP_GETTER(bool, includeSubtypes)
    UAPP_GETTER_WRAPPER(QualifiedName, targetName)
};

/**
 * UA_RelativePath wrapper class.
 */
class RelativePath : public WrapperNative<UA_RelativePath, UA_TYPES_RELATIVEPATH> {
public:
    using Wrapper::Wrapper;

    RelativePath(std::initializer_list<RelativePathElement> elements)
        : RelativePath({elements.begin(), elements.size()}) {}

    explicit RelativePath(Span<const RelativePathElement> elements) {
        handle()->elementsSize = elements.size();
        handle()->elements = detail::makeNativeArray(elements);
    }

    UAPP_GETTER_SPAN_WRAPPER(RelativePathElement, elements, elementsSize)
};

/**
 * UA_BrowsePath wrapper class.
 */
class BrowsePath : public WrapperNative<UA_BrowsePath, UA_TYPES_BROWSEPATH> {
public:
    using Wrapper::Wrapper;

    BrowsePath(NodeId startingNode, RelativePath relativePath) {
        handle()->startingNode = detail::makeNative(std::move(startingNode));
        handle()->relativePath = detail::makeNative(std::move(relativePath));
    }

    UAPP_GETTER_WRAPPER(NodeId, startingNode)
    UAPP_GETTER_WRAPPER(RelativePath, relativePath)
};

/**
 * UA_BrowsePathTarget wrapper class.
 */
class BrowsePathTarget : public WrapperNative<UA_BrowsePathTarget, UA_TYPES_BROWSEPATHTARGET> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ExpandedNodeId, targetId)
    UAPP_GETTER(uint32_t, remainingPathIndex)
};

/**
 * UA_BrowsePathResult wrapper class.
 */
class BrowsePathResult : public WrapperNative<UA_BrowsePathResult, UA_TYPES_BROWSEPATHRESULT> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER(StatusCode, statusCode)
    UAPP_GETTER_SPAN_WRAPPER(BrowsePathTarget, targets, targetsSize)
};

/**
 * UA_TranslateBrowsePathsToNodeIdsRequest wrapper class.
 */
class TranslateBrowsePathsToNodeIdsRequest
    : public WrapperNative<
          UA_TranslateBrowsePathsToNodeIdsRequest,
          UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST> {
public:
    using Wrapper::Wrapper;

    TranslateBrowsePathsToNodeIdsRequest(
        RequestHeader requestHeader, Span<const BrowsePath> browsePaths
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->browsePathsSize = browsePaths.size();
        handle()->browsePaths = detail::makeNativeArray(browsePaths);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(BrowsePath, browsePaths, browsePathsSize)
};

/**
 * UA_TranslateBrowsePathsToNodeIdsResponse wrapper class.
 */
class TranslateBrowsePathsToNodeIdsResponse
    : public WrapperNative<
          UA_TranslateBrowsePathsToNodeIdsResponse,
          UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(BrowsePathResult, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_RegisterNodesRequest wrapper class.
 */
class RegisterNodesRequest
    : public WrapperNative<UA_RegisterNodesRequest, UA_TYPES_REGISTERNODESREQUEST> {
public:
    using Wrapper::Wrapper;

    RegisterNodesRequest(RequestHeader requestHeader, Span<const NodeId> nodesToRegister) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->nodesToRegisterSize = nodesToRegister.size();
        handle()->nodesToRegister = detail::makeNativeArray(nodesToRegister);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(NodeId, nodesToRegister, nodesToRegisterSize)
};

/**
 * UA_RegisterNodesResponse wrapper class.
 */
class RegisterNodesResponse
    : public WrapperNative<UA_RegisterNodesResponse, UA_TYPES_REGISTERNODESRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(NodeId, registeredNodeIds, registeredNodeIdsSize)
};

/**
 * UA_UnregisterNodesRequest wrapper class.
 */
class UnregisterNodesRequest
    : public WrapperNative<UA_UnregisterNodesRequest, UA_TYPES_UNREGISTERNODESREQUEST> {
public:
    using Wrapper::Wrapper;

    UnregisterNodesRequest(RequestHeader requestHeader, Span<const NodeId> nodesToUnregister) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->nodesToUnregisterSize = nodesToUnregister.size();
        handle()->nodesToUnregister = detail::makeNativeArray(nodesToUnregister);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(NodeId, nodesToUnregister, nodesToUnregisterSize)
};

/**
 * UA_UnregisterNodesResponse wrapper class.
 */
class UnregisterNodesResponse
    : public WrapperNative<UA_UnregisterNodesResponse, UA_TYPES_UNREGISTERNODESRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
};

/**
 * Timestamps to return.
 * @see UA_TimestampsToReturn
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.40
 */
enum class TimestampsToReturn : int32_t {
    // clang-format off
    Source   = 0,
    Server   = 1,
    Both     = 2,
    Neither  = 3,
    Invalid  = 4,
    // clang-format on
};

/**
 * UA_ReadValueId wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.29
 */
class ReadValueId : public WrapperNative<UA_ReadValueId, UA_TYPES_READVALUEID> {
public:
    using Wrapper::Wrapper;

    ReadValueId(
        NodeId nodeId,
        AttributeId attributeId,
        std::string_view indexRange = {},
        QualifiedName dataEncoding = {}
    ) {
        handle()->nodeId = detail::makeNative(std::move(nodeId));
        handle()->attributeId = detail::makeNative(attributeId);
        handle()->indexRange = detail::makeNative(indexRange);
        handle()->dataEncoding = detail::makeNative(std::move(dataEncoding));
    }

    UAPP_GETTER_WRAPPER(NodeId, nodeId)
    UAPP_GETTER_CAST(AttributeId, attributeId)
    UAPP_GETTER_WRAPPER(String, indexRange)
    UAPP_GETTER_WRAPPER(QualifiedName, dataEncoding)
};

/**
 * UA_ReadRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.2
 */
class ReadRequest : public WrapperNative<UA_ReadRequest, UA_TYPES_READREQUEST> {
public:
    using Wrapper::Wrapper;

    ReadRequest(
        RequestHeader requestHeader,
        double maxAge,
        TimestampsToReturn timestampsToReturn,
        Span<const ReadValueId> nodesToRead
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->maxAge = maxAge;
        handle()->timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestampsToReturn);
        handle()->nodesToReadSize = nodesToRead.size();
        handle()->nodesToRead = detail::makeNativeArray(nodesToRead);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER(double, maxAge)
    UAPP_GETTER_CAST(TimestampsToReturn, timestampsToReturn)
    UAPP_GETTER_SPAN_WRAPPER(ReadValueId, nodesToRead, nodesToReadSize)
};

/**
 * UA_ReadResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.2
 */
class ReadResponse : public WrapperNative<UA_ReadResponse, UA_TYPES_READRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(DataValue, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_WriteValue wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.4
 */
class WriteValue : public WrapperNative<UA_WriteValue, UA_TYPES_WRITEVALUE> {
public:
    using Wrapper::Wrapper;

    WriteValue(
        NodeId nodeId, AttributeId attributeId, std::string_view indexRange, DataValue value
    ) {
        handle()->nodeId = detail::makeNative(std::move(nodeId));
        handle()->attributeId = detail::makeNative(attributeId);
        handle()->indexRange = detail::makeNative(indexRange);
        handle()->value = detail::makeNative(std::move(value));
    }

    UAPP_GETTER_WRAPPER(NodeId, nodeId)
    UAPP_GETTER_CAST(AttributeId, attributeId)
    UAPP_GETTER_WRAPPER(String, indexRange)
    UAPP_GETTER_WRAPPER(DataValue, value)
};

/**
 * UA_WriteRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.4
 */
class WriteRequest : public WrapperNative<UA_WriteRequest, UA_TYPES_WRITEREQUEST> {
public:
    using Wrapper::Wrapper;

    WriteRequest(RequestHeader requestHeader, Span<const WriteValue> nodesToWrite) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->nodesToWriteSize = nodesToWrite.size();
        handle()->nodesToWrite = detail::makeNativeArray(nodesToWrite);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(WriteValue, nodesToWrite, nodesToWriteSize)
};

/**
 * UA_WriteResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.4
 */
class WriteResponse : public WrapperNative<UA_WriteResponse, UA_TYPES_WRITERESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_BuildInfo wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part5/v105/docs/12.4
 */
class BuildInfo : public WrapperNative<UA_BuildInfo, UA_TYPES_BUILDINFO> {
public:
    using Wrapper::Wrapper;

    BuildInfo(
        std::string_view productUri,
        std::string_view manufacturerName,
        std::string_view productName,
        std::string_view softwareVersion,
        std::string_view buildNumber,
        DateTime buildDate
    ) {
        handle()->productUri = detail::makeNative(productUri);
        handle()->manufacturerName = detail::makeNative(manufacturerName);
        handle()->productName = detail::makeNative(productName);
        handle()->softwareVersion = detail::makeNative(softwareVersion);
        handle()->buildNumber = detail::makeNative(buildNumber);
        handle()->buildDate = detail::makeNative(std::move(buildDate));
    }

    UAPP_GETTER_WRAPPER(String, productUri);
    UAPP_GETTER_WRAPPER(String, manufacturerName);
    UAPP_GETTER_WRAPPER(String, productName);
    UAPP_GETTER_WRAPPER(String, softwareVersion);
    UAPP_GETTER_WRAPPER(String, buildNumber);
    UAPP_GETTER_WRAPPER(DateTime, buildDate)
};

/* ------------------------------------------- Method ------------------------------------------- */

#ifdef UA_ENABLE_METHODCALLS

/**
 * UA_Argument wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.6
 */
class Argument : public WrapperNative<UA_Argument, UA_TYPES_ARGUMENT> {
public:
    using Wrapper::Wrapper;

    Argument(
        std::string_view name,
        LocalizedText description,
        NodeId dataType,
        ValueRank valueRank = {},
        Span<const uint32_t> arrayDimensions = {}
    ) {
        handle()->name = detail::makeNative(name);
        handle()->description = detail::makeNative(std::move(description));
        handle()->dataType = detail::makeNative(std::move(dataType));
        handle()->valueRank = detail::makeNative(valueRank);
        handle()->arrayDimensionsSize = arrayDimensions.size();
        handle()->arrayDimensions = detail::makeNativeArray(arrayDimensions);
    }

    UAPP_GETTER_WRAPPER(String, name)
    UAPP_GETTER_WRAPPER(LocalizedText, description)
    UAPP_GETTER_WRAPPER(NodeId, dataType)
    UAPP_GETTER_CAST(ValueRank, valueRank)
    UAPP_GETTER_SPAN(uint32_t, arrayDimensions, arrayDimensionsSize)
};

/**
 * UA_CallMethodRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11.2
 */
class CallMethodRequest : public WrapperNative<UA_CallMethodRequest, UA_TYPES_CALLMETHODREQUEST> {
public:
    using Wrapper::Wrapper;

    CallMethodRequest(NodeId objectId, NodeId methodId, Span<const Variant> inputArguments) {
        handle()->objectId = detail::makeNative(std::move(objectId));
        handle()->methodId = detail::makeNative(std::move(methodId));
        handle()->inputArgumentsSize = inputArguments.size();
        handle()->inputArguments = detail::makeNativeArray(inputArguments);
    }

    UAPP_GETTER_WRAPPER(NodeId, objectId)
    UAPP_GETTER_WRAPPER(NodeId, methodId)
    UAPP_GETTER_SPAN_WRAPPER(Variant, inputArguments, inputArgumentsSize)
};

/**
 * UA_CallMethodResult wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11.2
 */
class CallMethodResult : public WrapperNative<UA_CallMethodResult, UA_TYPES_CALLMETHODRESULT> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(StatusCode, statusCode)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, inputArgumentResults, inputArgumentResultsSize)
    UAPP_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, inputArgumentDiagnosticInfos, inputArgumentDiagnosticInfosSize
    )
    UAPP_GETTER_SPAN_WRAPPER(Variant, outputArguments, outputArgumentsSize)
};

/**
 * UA_CallRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11.2
 */
class CallRequest : public WrapperNative<UA_CallRequest, UA_TYPES_CALLREQUEST> {
public:
    using Wrapper::Wrapper;

    CallRequest(RequestHeader requestHeader, Span<const CallMethodRequest> methodsToCall) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->methodsToCallSize = methodsToCall.size();
        handle()->methodsToCall = detail::makeNativeArray(methodsToCall);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER_SPAN_WRAPPER(CallMethodRequest, methodsToCall, methodsToCallSize)
};

/**
 * UA_CallResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.11.2
 */
class CallResponse : public WrapperNative<UA_CallResponse, UA_TYPES_CALLRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(CallMethodResult, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

#endif  // UA_ENABLE_METHODCALLS

/* ---------------------------------------- Subscriptions --------------------------------------- */

#ifdef UA_ENABLE_SUBSCRIPTIONS

/**
 * Monitoring mode.
 * @see UA_MonitoringMode
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.23
 */
enum class MonitoringMode : int32_t {
    // clang-format off
    Disabled  = 0,
    Sampling  = 1,
    Reporting = 2,
    // clang-format on
};

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
class ElementOperand : public WrapperNative<UA_ElementOperand, UA_TYPES_ELEMENTOPERAND> {
public:
    using Wrapper::Wrapper;

    explicit ElementOperand(uint32_t index) {
        handle()->index = index;
    }

    UAPP_GETTER(uint32_t, index)
};

/**
 * UA_LiteralOperand wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.4.3
 */
class LiteralOperand : public WrapperNative<UA_LiteralOperand, UA_TYPES_LITERALOPERAND> {
private:
    template <typename T>
    using EnableIfLiteral =
        std::enable_if_t<!detail::IsOneOf<T, Variant, UA_LiteralOperand, LiteralOperand>::value>;

public:
    using Wrapper::Wrapper;

    explicit LiteralOperand(Variant value) {
        handle()->value = detail::makeNative(std::move(value));
    }

    template <typename T, typename = EnableIfLiteral<T>>
    explicit LiteralOperand(T&& literal)
        : LiteralOperand{Variant{std::forward<T>(literal)}} {}

    UAPP_GETTER_WRAPPER(Variant, value)
};

/**
 * UA_AttributeOperand wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.4.4
 */
class AttributeOperand : public WrapperNative<UA_AttributeOperand, UA_TYPES_ATTRIBUTEOPERAND> {
public:
    using Wrapper::Wrapper;

    AttributeOperand(
        NodeId nodeId,
        std::string_view alias,
        RelativePath browsePath,
        AttributeId attributeId,
        std::string_view indexRange = {}
    ) {
        handle()->nodeId = detail::makeNative(std::move(nodeId));
        handle()->alias = detail::makeNative(alias);
        handle()->browsePath = detail::makeNative(std::move(browsePath));
        handle()->attributeId = detail::makeNative(attributeId);
        handle()->indexRange = detail::makeNative(indexRange);
    }

    UAPP_GETTER_WRAPPER(NodeId, nodeId)
    UAPP_GETTER_WRAPPER(String, alias)
    UAPP_GETTER_WRAPPER(RelativePath, browsePath)
    UAPP_GETTER_CAST(AttributeId, attributeId)
    UAPP_GETTER_WRAPPER(String, indexRange)
};

/**
 * UA_SimpleAttributeOperand wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.4.5
 */
class SimpleAttributeOperand
    : public WrapperNative<UA_SimpleAttributeOperand, UA_TYPES_SIMPLEATTRIBUTEOPERAND> {
public:
    using Wrapper::Wrapper;

    SimpleAttributeOperand(
        NodeId typeDefinitionId,
        Span<const QualifiedName> browsePath,
        AttributeId attributeId,
        std::string_view indexRange = {}
    ) {
        handle()->typeDefinitionId = detail::makeNative(std::move(typeDefinitionId));
        handle()->browsePathSize = browsePath.size();
        handle()->browsePath = detail::makeNativeArray(browsePath);
        handle()->attributeId = detail::makeNative(attributeId);
        handle()->indexRange = detail::makeNative(indexRange);
    }

    UAPP_GETTER_WRAPPER(NodeId, typeDefinitionId)
    UAPP_GETTER_SPAN_WRAPPER(QualifiedName, browsePath, browsePathSize)
    UAPP_GETTER_CAST(AttributeId, attributeId)
    UAPP_GETTER_WRAPPER(String, indexRange)
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
    : public WrapperNative<UA_ContentFilterElement, UA_TYPES_CONTENTFILTERELEMENT> {
public:
    using Wrapper::Wrapper;

    ContentFilterElement(FilterOperator filterOperator, Span<const FilterOperand> operands);

    UAPP_GETTER_CAST(FilterOperator, filterOperator)
    UAPP_GETTER_SPAN_WRAPPER(ExtensionObject, filterOperands, filterOperandsSize)
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
class ContentFilter : public WrapperNative<UA_ContentFilter, UA_TYPES_CONTENTFILTER> {
public:
    using Wrapper::Wrapper;

    ContentFilter(std::initializer_list<ContentFilterElement> elements);
    explicit ContentFilter(Span<const ContentFilterElement> elements);

    UAPP_GETTER_SPAN_WRAPPER(ContentFilterElement, elements, elementsSize)
};

/// @relates ContentFilterElement
ContentFilter operator!(const ContentFilterElement& filterElement);
/// @relates ContentFilter
ContentFilter operator!(const ContentFilter& filter);

/// @relates ContentFilterElement
ContentFilter operator&&(const ContentFilterElement& lhs, const ContentFilterElement& rhs);
/// @relates ContentFilterElement
/// @relatesalso ContentFilter
ContentFilter operator&&(const ContentFilterElement& lhs, const ContentFilter& rhs);
/// @relates ContentFilter
/// @relatesalso ContentFilterElement
ContentFilter operator&&(const ContentFilter& lhs, const ContentFilterElement& rhs);
/// @relates ContentFilter
ContentFilter operator&&(const ContentFilter& lhs, const ContentFilter& rhs);

/// @relates ContentFilterElement
ContentFilter operator||(const ContentFilterElement& lhs, const ContentFilterElement& rhs);
/// @relates ContentFilterElement
/// @relatesalso ContentFilter
ContentFilter operator||(const ContentFilterElement& lhs, const ContentFilter& rhs);
/// @relates ContentFilter
/// @relatesalso ContentFilterElement
ContentFilter operator||(const ContentFilter& lhs, const ContentFilterElement& rhs);
/// @relates ContentFilter
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
class DataChangeFilter : public WrapperNative<UA_DataChangeFilter, UA_TYPES_DATACHANGEFILTER> {
public:
    using Wrapper::Wrapper;

    DataChangeFilter(DataChangeTrigger trigger, DeadbandType deadbandType, double deadbandValue) {
        handle()->trigger = static_cast<UA_DataChangeTrigger>(trigger);
        handle()->deadbandType = detail::makeNative(deadbandType);
        handle()->deadbandValue = deadbandValue;
    }

    UAPP_GETTER_CAST(DataChangeTrigger, trigger)
    UAPP_GETTER_CAST(DeadbandType, deadbandType)
    UAPP_GETTER(double, deadbandValue)
};

/**
 * UA_EventFilter wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.22.3
 */
class EventFilter : public WrapperNative<UA_EventFilter, UA_TYPES_EVENTFILTER> {
public:
    using Wrapper::Wrapper;

    EventFilter(Span<const SimpleAttributeOperand> selectClauses, ContentFilter whereClause) {
        handle()->selectClausesSize = selectClauses.size();
        handle()->selectClauses = detail::makeNativeArray(selectClauses);
        handle()->whereClause = detail::makeNative(std::move(whereClause));
    }

    UAPP_GETTER_SPAN_WRAPPER(SimpleAttributeOperand, selectClauses, selectClausesSize)
    UAPP_GETTER_WRAPPER(ContentFilter, whereClause)
};

using AggregateConfiguration = UA_AggregateConfiguration;

/**
 * UA_AggregateFilter wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.22.4
 */
class AggregateFilter : public WrapperNative<UA_AggregateFilter, UA_TYPES_AGGREGATEFILTER> {
public:
    using Wrapper::Wrapper;

    AggregateFilter(
        DateTime startTime,
        NodeId aggregateType,
        double processingInterval,
        AggregateConfiguration aggregateConfiguration
    ) {
        handle()->startTime = detail::makeNative(std::move(startTime));
        handle()->aggregateType = detail::makeNative(std::move(aggregateType));
        handle()->processingInterval = processingInterval;
        handle()->aggregateConfiguration = aggregateConfiguration;  // TODO: make wrapper?
    }

    UAPP_GETTER_WRAPPER(DateTime, startTime)
    UAPP_GETTER_WRAPPER(NodeId, aggregateType)
    UAPP_GETTER(double, processingInterval)
    UAPP_GETTER(AggregateConfiguration, aggregateConfiguration)
};

/**
 * UA_MonitoringParameters wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.21
 */
class MonitoringParameters
    : public WrapperNative<UA_MonitoringParameters, UA_TYPES_MONITORINGPARAMETERS> {
public:
    using Wrapper::Wrapper;

    /// Construct with default values from open62541.
    /// The `clientHandle` parameter cannot be set by the user, any value will be replaced by the
    /// client before sending the request to the server.
    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    MonitoringParameters(
        double samplingInterval = 250.0,
        ExtensionObject filter = {},
        uint32_t queueSize = 1U,
        bool discardOldest = true
    ) {
        handle()->samplingInterval = samplingInterval;
        handle()->filter = detail::makeNative(std::move(filter));
        handle()->queueSize = queueSize;
        handle()->discardOldest = discardOldest;
    }

    UAPP_GETTER(double, samplingInterval)
    UAPP_GETTER_WRAPPER(ExtensionObject, filter)
    UAPP_GETTER(uint32_t, queueSize)
    UAPP_GETTER(bool, discardOldest)
};

/**
 * UA_MonitoredItemCreateRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.2
 */
class MonitoredItemCreateRequest
    : public WrapperNative<UA_MonitoredItemCreateRequest, UA_TYPES_MONITOREDITEMCREATEREQUEST> {
public:
    using Wrapper::Wrapper;

    explicit MonitoredItemCreateRequest(
        ReadValueId itemToMonitor,
        MonitoringMode monitoringMode = MonitoringMode::Reporting,
        MonitoringParameters requestedParameters = {}
    ) {
        handle()->itemToMonitor = detail::makeNative(std::move(itemToMonitor));
        handle()->monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
        handle()->requestedParameters = detail::makeNative(std::move(requestedParameters));
    }

    UAPP_GETTER_WRAPPER(ReadValueId, itemToMonitor)
    UAPP_GETTER_CAST(MonitoringMode, monitoringMode)
    UAPP_GETTER_WRAPPER(MonitoringParameters, requestedParameters)
};

/**
 * UA_MonitoredItemCreateResult wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.2
 */
class MonitoredItemCreateResult
    : public WrapperNative<UA_MonitoredItemCreateResult, UA_TYPES_MONITOREDITEMCREATERESULT> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(StatusCode, statusCode);
    UAPP_GETTER(IntegerId, monitoredItemId);
    UAPP_GETTER(double, revisedSamplingInterval);
    UAPP_GETTER(IntegerId, revisedQueueSize);
    UAPP_GETTER_WRAPPER(ExtensionObject, filterResult);
};

/**
 * UA_CreateMonitoredItemsRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.2
 */
class CreateMonitoredItemsRequest
    : public WrapperNative<UA_CreateMonitoredItemsRequest, UA_TYPES_CREATEMONITOREDITEMSREQUEST> {
public:
    using Wrapper::Wrapper;

    CreateMonitoredItemsRequest(
        RequestHeader requestHeader,
        IntegerId subscriptionId,
        TimestampsToReturn timestampsToReturn,
        Span<const MonitoredItemCreateRequest> itemsToCreate
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->subscriptionId = subscriptionId;
        handle()->timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestampsToReturn);
        handle()->itemsToCreateSize = itemsToCreate.size();
        handle()->itemsToCreate = detail::makeNativeArray(itemsToCreate);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER(IntegerId, subscriptionId)
    UAPP_GETTER_CAST(TimestampsToReturn, timestampsToReturn)
    UAPP_GETTER_SPAN_WRAPPER(MonitoredItemCreateRequest, itemsToCreate, itemsToCreateSize)
};

/**
 * UA_CreateMonitoredItemsResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.2
 */
class CreateMonitoredItemsResponse
    : public WrapperNative<UA_CreateMonitoredItemsResponse, UA_TYPES_CREATEMONITOREDITEMSRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(MonitoredItemCreateResult, results, resultsSize);
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_MonitoredItemModifyRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.3
 */
class MonitoredItemModifyRequest
    : public WrapperNative<UA_MonitoredItemModifyRequest, UA_TYPES_MONITOREDITEMMODIFYREQUEST> {
public:
    using Wrapper::Wrapper;

    MonitoredItemModifyRequest(
        IntegerId monitoredItemId, MonitoringParameters requestedParameters
    ) {
        handle()->monitoredItemId = monitoredItemId;
        handle()->requestedParameters = detail::makeNative(std::move(requestedParameters));
    }

    UAPP_GETTER(IntegerId, monitoredItemId);
    UAPP_GETTER_WRAPPER(MonitoringParameters, requestedParameters)
};

/**
 * UA_MonitoredItemModifyResult wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.3
 */
class MonitoredItemModifyResult
    : public WrapperNative<UA_MonitoredItemModifyResult, UA_TYPES_MONITOREDITEMMODIFYRESULT> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(StatusCode, statusCode);
    UAPP_GETTER(double, revisedSamplingInterval);
    UAPP_GETTER(uint32_t, revisedQueueSize);
    UAPP_GETTER_WRAPPER(ExtensionObject, filterResult);
};

/**
 * UA_ModifyMonitoredItemsRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.3
 */
class ModifyMonitoredItemsRequest
    : public WrapperNative<UA_ModifyMonitoredItemsRequest, UA_TYPES_MODIFYMONITOREDITEMSREQUEST> {
public:
    using Wrapper::Wrapper;

    ModifyMonitoredItemsRequest(
        RequestHeader requestHeader,
        IntegerId subscriptionId,
        TimestampsToReturn timestampsToReturn,
        Span<const MonitoredItemModifyRequest> itemsToModify
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->subscriptionId = subscriptionId;
        handle()->timestampsToReturn = static_cast<UA_TimestampsToReturn>(timestampsToReturn);
        handle()->itemsToModifySize = itemsToModify.size();
        handle()->itemsToModify = detail::makeNativeArray(itemsToModify);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER(IntegerId, subscriptionId)
    UAPP_GETTER_CAST(TimestampsToReturn, timestampsToReturn)
    UAPP_GETTER_SPAN_WRAPPER(MonitoredItemModifyRequest, itemsToModify, itemsToModifySize)
};

/**
 * UA_CreateMonitoredItemsResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.3
 */
class ModifyMonitoredItemsResponse
    : public WrapperNative<UA_ModifyMonitoredItemsResponse, UA_TYPES_MODIFYMONITOREDITEMSRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(MonitoredItemModifyResult, results, resultsSize);
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_SetMonitoringModeRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.4
 */
class SetMonitoringModeRequest
    : public WrapperNative<UA_SetMonitoringModeRequest, UA_TYPES_SETMONITORINGMODEREQUEST> {
public:
    using Wrapper::Wrapper;

    SetMonitoringModeRequest(
        RequestHeader requestHeader,
        IntegerId subscriptionId,
        MonitoringMode monitoringMode,
        Span<const IntegerId> monitoredItemIds
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->subscriptionId = subscriptionId;
        handle()->monitoringMode = static_cast<UA_MonitoringMode>(monitoringMode);
        handle()->monitoredItemIdsSize = monitoredItemIds.size();
        handle()->monitoredItemIds = detail::makeNativeArray(monitoredItemIds);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER(IntegerId, subscriptionId)
    UAPP_GETTER_CAST(MonitoringMode, monitoringMode)
    UAPP_GETTER_SPAN(IntegerId, monitoredItemIds, monitoredItemIdsSize)
};

/**
 * UA_SetMonitoringModeResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.4
 */
class SetMonitoringModeResponse
    : public WrapperNative<UA_SetMonitoringModeResponse, UA_TYPES_SETMONITORINGMODERESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, results, resultsSize);
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_SetTriggeringRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.5
 */
class SetTriggeringRequest
    : public WrapperNative<UA_SetTriggeringRequest, UA_TYPES_SETTRIGGERINGREQUEST> {
public:
    using Wrapper::Wrapper;

    SetTriggeringRequest(
        RequestHeader requestHeader,
        IntegerId subscriptionId,
        IntegerId triggeringItemId,
        Span<const IntegerId> linksToAdd,
        Span<const IntegerId> linksToRemove
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->subscriptionId = subscriptionId;
        handle()->triggeringItemId = triggeringItemId;
        handle()->linksToAddSize = linksToAdd.size();
        handle()->linksToAdd = detail::makeNativeArray(linksToAdd);
        handle()->linksToRemoveSize = linksToRemove.size();
        handle()->linksToRemove = detail::makeNativeArray(linksToRemove);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER(IntegerId, subscriptionId)
    UAPP_GETTER(IntegerId, triggeringItemId)
    UAPP_GETTER_SPAN(IntegerId, linksToAdd, linksToAddSize)
    UAPP_GETTER_SPAN(IntegerId, linksToRemove, linksToRemoveSize)
};

/**
 * UA_SetTriggeringResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.5
 */
class SetTriggeringResponse
    : public WrapperNative<UA_SetTriggeringResponse, UA_TYPES_SETTRIGGERINGRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, addResults, addResultsSize);
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, addDiagnosticInfos, addDiagnosticInfosSize)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, removeResults, removeResultsSize);
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, removeDiagnosticInfos, removeDiagnosticInfosSize)
};

/**
 * UA_DeleteMonitoredItemsRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.6
 */
class DeleteMonitoredItemsRequest
    : public WrapperNative<UA_DeleteMonitoredItemsRequest, UA_TYPES_DELETEMONITOREDITEMSREQUEST> {
public:
    using Wrapper::Wrapper;

    DeleteMonitoredItemsRequest(
        RequestHeader requestHeader,
        IntegerId subscriptionId,
        Span<const IntegerId> monitoredItemIds
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->subscriptionId = subscriptionId;
        handle()->monitoredItemIdsSize = monitoredItemIds.size();
        handle()->monitoredItemIds = detail::makeNativeArray(monitoredItemIds);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER(IntegerId, subscriptionId)
    UAPP_GETTER_SPAN(IntegerId, monitoredItemIds, monitoredItemIdsSize)
};

/**
 * UA_DeleteMonitoredItemsResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.6
 */
class DeleteMonitoredItemsResponse
    : public WrapperNative<UA_DeleteMonitoredItemsResponse, UA_TYPES_DELETEMONITOREDITEMSRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, results, resultsSize);
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_CreateSubscriptionRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.2
 */
class CreateSubscriptionRequest
    : public WrapperNative<UA_CreateSubscriptionRequest, UA_TYPES_CREATESUBSCRIPTIONREQUEST> {
public:
    using Wrapper::Wrapper;

    CreateSubscriptionRequest(
        RequestHeader requestHeader,
        double requestedPublishingInterval,
        uint32_t requestedLifetimeCount,
        uint32_t requestedMaxKeepAliveCount,
        uint32_t maxNotificationsPerPublish,
        bool publishingEnabled,
        uint8_t priority
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->requestedPublishingInterval = requestedPublishingInterval;
        handle()->requestedLifetimeCount = requestedLifetimeCount;
        handle()->requestedMaxKeepAliveCount = requestedMaxKeepAliveCount;
        handle()->maxNotificationsPerPublish = maxNotificationsPerPublish;
        handle()->publishingEnabled = publishingEnabled;
        handle()->priority = priority;
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER(double, requestedPublishingInterval)
    UAPP_GETTER(uint32_t, requestedLifetimeCount)
    UAPP_GETTER(uint32_t, requestedMaxKeepAliveCount)
    UAPP_GETTER(uint32_t, maxNotificationsPerPublish)
    UAPP_GETTER(bool, publishingEnabled)
    UAPP_GETTER(uint8_t, priority)
};

/**
 * UA_CreateSubscriptionResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.2
 */
class CreateSubscriptionResponse
    : public WrapperNative<UA_CreateSubscriptionResponse, UA_TYPES_CREATESUBSCRIPTIONRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER(IntegerId, subscriptionId)
    UAPP_GETTER(bool, revisedPublishingInterval)
    UAPP_GETTER(uint32_t, revisedLifetimeCount)
    UAPP_GETTER(uint32_t, revisedMaxKeepAliveCount)
};

/**
 * UA_ModifySubscriptionRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.3
 */
class ModifySubscriptionRequest
    : public WrapperNative<UA_ModifySubscriptionRequest, UA_TYPES_MODIFYSUBSCRIPTIONREQUEST> {
public:
    using Wrapper::Wrapper;

    ModifySubscriptionRequest(
        RequestHeader requestHeader,
        IntegerId subscriptionId,
        double requestedPublishingInterval,
        uint32_t requestedLifetimeCount,
        uint32_t requestedMaxKeepAliveCount,
        uint32_t maxNotificationsPerPublish,
        uint8_t priority
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->subscriptionId = subscriptionId;
        handle()->requestedPublishingInterval = requestedPublishingInterval;
        handle()->requestedLifetimeCount = requestedLifetimeCount;
        handle()->requestedMaxKeepAliveCount = requestedMaxKeepAliveCount;
        handle()->maxNotificationsPerPublish = maxNotificationsPerPublish;
        handle()->priority = priority;
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER(IntegerId, subscriptionId)
    UAPP_GETTER(double, requestedPublishingInterval)
    UAPP_GETTER(uint32_t, requestedLifetimeCount)
    UAPP_GETTER(uint32_t, requestedMaxKeepAliveCount)
    UAPP_GETTER(uint32_t, maxNotificationsPerPublish)
    UAPP_GETTER(uint8_t, priority)
};

/**
 * UA_ModifySubscriptionResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.3
 */
class ModifySubscriptionResponse
    : public WrapperNative<UA_ModifySubscriptionResponse, UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER(bool, revisedPublishingInterval)
    UAPP_GETTER(uint32_t, revisedLifetimeCount)
    UAPP_GETTER(uint32_t, revisedMaxKeepAliveCount)
};

/**
 * UA_SetPublishingModeRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.4
 */
class SetPublishingModeRequest
    : public WrapperNative<UA_SetPublishingModeRequest, UA_TYPES_SETPUBLISHINGMODEREQUEST> {
public:
    using Wrapper::Wrapper;

    SetPublishingModeRequest(
        RequestHeader requestHeader, bool publishingEnabled, Span<const IntegerId> subscriptionIds
    ) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->publishingEnabled = publishingEnabled;
        handle()->subscriptionIdsSize = subscriptionIds.size();
        handle()->subscriptionIds = detail::makeNativeArray(subscriptionIds);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER(bool, publishingEnabled)
    UAPP_GETTER_SPAN(IntegerId, subscriptionIds, subscriptionIdsSize)
};

/**
 * UA_SetPublishingModeResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.4
 */
class SetPublishingModeResponse
    : public WrapperNative<UA_SetPublishingModeResponse, UA_TYPES_SETPUBLISHINGMODERESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

/**
 * UA_StatusChangeNotification wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.25.4
 */
class StatusChangeNotification
    : public WrapperNative<UA_StatusChangeNotification, UA_TYPES_STATUSCHANGENOTIFICATION> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(StatusCode, status)
    UAPP_GETTER_WRAPPER(DiagnosticInfo, diagnosticInfo)
};

/**
 * UA_DeleteSubscriptionsRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.8
 */
class DeleteSubscriptionsRequest
    : public WrapperNative<UA_DeleteSubscriptionsRequest, UA_TYPES_DELETESUBSCRIPTIONSREQUEST> {
public:
    using Wrapper::Wrapper;

    DeleteSubscriptionsRequest(RequestHeader requestHeader, Span<const IntegerId> subscriptionIds) {
        handle()->requestHeader = detail::makeNative(std::move(requestHeader));
        handle()->subscriptionIdsSize = subscriptionIds.size();
        handle()->subscriptionIds = detail::makeNativeArray(subscriptionIds);
    }

    UAPP_GETTER_WRAPPER(RequestHeader, requestHeader)
    UAPP_GETTER_SPAN(IntegerId, subscriptionIds, subscriptionIdsSize)
};

/**
 * UA_DeleteSubscriptionsResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.13.8
 */
class DeleteSubscriptionsResponse
    : public WrapperNative<UA_DeleteSubscriptionsResponse, UA_TYPES_DELETESUBSCRIPTIONSRESPONSE> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(ResponseHeader, responseHeader)
    UAPP_GETTER_SPAN_WRAPPER(StatusCode, results, resultsSize)
    UAPP_GETTER_SPAN_WRAPPER(DiagnosticInfo, diagnosticInfos, diagnosticInfosSize)
};

#endif  // UA_ENABLE_SUBSCRIPTIONS

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

/* ----------------------------------------- DataAccess ----------------------------------------- */

#if UAPP_HAS_DATAACCESS

/**
 * UA_Range wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part8/v105/docs/5.6.2
 */
class Range : public WrapperNative<UA_Range, UA_TYPES_RANGE> {
public:
    using Wrapper::Wrapper;

    Range(double low, double high) noexcept {
        handle()->low = low;
        handle()->high = high;
    }

    UAPP_GETTER(double, low)
    UAPP_GETTER(double, high)
};

/**
 * UA_EUInformation wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part8/v105/docs/5.6.3
 */
class EUInformation : public WrapperNative<UA_EUInformation, UA_TYPES_EUINFORMATION> {
public:
    using Wrapper::Wrapper;

    EUInformation(
        std::string_view namespaceUri,
        int32_t unitId,
        LocalizedText displayName,
        LocalizedText description
    ) {
        handle()->namespaceUri = detail::makeNative(namespaceUri);
        handle()->unitId = unitId;
        handle()->displayName = detail::makeNative(std::move(displayName));
        handle()->description = detail::makeNative(std::move(description));
    }

    UAPP_GETTER_WRAPPER(String, namespaceUri)
    UAPP_GETTER(int32_t, unitId)
    UAPP_GETTER_WRAPPER(LocalizedText, displayName)
    UAPP_GETTER_WRAPPER(LocalizedText, description)
};

/**
 * UA_ComplexNumberType wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part8/v105/docs/5.6.4
 */
class ComplexNumberType : public WrapperNative<UA_ComplexNumberType, UA_TYPES_COMPLEXNUMBERTYPE> {
public:
    using Wrapper::Wrapper;

    ComplexNumberType(float real, float imaginary) noexcept {
        handle()->real = real;
        handle()->imaginary = imaginary;
    }

    UAPP_GETTER(float, real)
    UAPP_GETTER(float, imaginary)
};

/**
 * UA_DoubleComplexNumberType wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part8/v105/docs/5.6.5
 */
class DoubleComplexNumberType
    : public WrapperNative<UA_DoubleComplexNumberType, UA_TYPES_DOUBLECOMPLEXNUMBERTYPE> {
public:
    using Wrapper::Wrapper;

    DoubleComplexNumberType(double real, double imaginary) noexcept {
        handle()->real = real;
        handle()->imaginary = imaginary;
    }

    UAPP_GETTER(double, real)
    UAPP_GETTER(double, imaginary)
};

/**
 * Axis scale.
 * @see https://reference.opcfoundation.org/Core/Part8/v105/docs/5.6.7
 */
enum class AxisScaleEnumeration : int32_t {
    Linear = 1,
    Log = 2,
    Ln = 3,
};

/**
 * UA_AxisInformation wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part8/v105/docs/5.6.6
 */
class AxisInformation : public WrapperNative<UA_AxisInformation, UA_TYPES_AXISINFORMATION> {
public:
    using Wrapper::Wrapper;

    AxisInformation(
        EUInformation engineeringUnits,
        Range eURange,
        LocalizedText title,
        AxisScaleEnumeration axisScaleType,
        Span<const double> axisSteps
    ) {
        handle()->engineeringUnits = detail::makeNative(std::move(engineeringUnits));
        handle()->eURange = detail::makeNative(std::move(eURange));
        handle()->title = detail::makeNative(std::move(title));
        handle()->axisScaleType = static_cast<UA_AxisScaleEnumeration>(axisScaleType);
        handle()->axisStepsSize = axisSteps.size();
        handle()->axisSteps = detail::makeNativeArray(axisSteps);
    }

    UAPP_GETTER_WRAPPER(EUInformation, engineeringUnits)
    UAPP_GETTER_WRAPPER(Range, eURange)
    UAPP_GETTER_WRAPPER(LocalizedText, title)
    UAPP_GETTER_CAST(AxisScaleEnumeration, axisScaleType)
    UAPP_GETTER_SPAN(double, axisSteps, axisStepsSize)
};

/**
 * UA_XVType wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part8/v105/docs/5.6.8
 */
class XVType : public WrapperNative<UA_XVType, UA_TYPES_XVTYPE> {
public:
    using Wrapper::Wrapper;

    XVType(double x, float value) noexcept {
        handle()->x = x;
        handle()->value = value;
    }

    UAPP_GETTER(double, x)
    UAPP_GETTER(float, value)
};

#endif  // UAPP_HAS_DATAACCESS

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
class StructureField : public WrapperNative<UA_StructureField, UA_TYPES_STRUCTUREFIELD> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(String, name)
    UAPP_GETTER_WRAPPER(LocalizedText, description)
    UAPP_GETTER_WRAPPER(NodeId, dataType)
    UAPP_GETTER_CAST(ValueRank, valueRank)
    UAPP_GETTER_SPAN(uint32_t, arrayDimensions, arrayDimensionsSize)
    UAPP_GETTER(uint32_t, maxStringLength)
    UAPP_GETTER(bool, isOptional)
};

/**
 * UA_StructureDefinition wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.48
 */
class StructureDefinition
    : public WrapperNative<UA_StructureDefinition, UA_TYPES_STRUCTUREDEFINITION> {
public:
    using Wrapper::Wrapper;

    UAPP_GETTER_WRAPPER(NodeId, defaultEncodingId)
    UAPP_GETTER_WRAPPER(NodeId, baseDataType)
    UAPP_GETTER_CAST(StructureType, structureType)
    UAPP_GETTER_SPAN_WRAPPER(StructureField, fields, fieldsSize)
};

/**
 * UA_EnumField wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.52
 */
class EnumField : public WrapperNative<UA_EnumField, UA_TYPES_ENUMFIELD> {
public:
    using Wrapper::Wrapper;

    EnumField(int64_t value, std::string_view name)
        : EnumField(value, {"", name}, {}, name) {}

    EnumField(
        int64_t value, LocalizedText displayName, LocalizedText description, std::string_view name
    ) {
        handle()->value = value;
        handle()->displayName = detail::makeNative(std::move(displayName));
        handle()->description = detail::makeNative(std::move(description));
        handle()->name = detail::makeNative(name);
    }

    UAPP_GETTER(int64_t, value)
    UAPP_GETTER_WRAPPER(LocalizedText, displayName)
    UAPP_GETTER_WRAPPER(LocalizedText, description)
    UAPP_GETTER_WRAPPER(String, name)
};

/**
 * UA_EnumDefinition wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.50
 */
class EnumDefinition : public WrapperNative<UA_EnumDefinition, UA_TYPES_ENUMDEFINITION> {
public:
    using Wrapper::Wrapper;

    EnumDefinition(std::initializer_list<EnumField> fields)
        : EnumDefinition({fields.begin(), fields.size()}) {}

    explicit EnumDefinition(Span<const EnumField> fields) {
        handle()->fieldsSize = fields.size();
        handle()->fields = detail::makeNativeArray(fields);
    }

    UAPP_GETTER_SPAN_WRAPPER(EnumField, fields, fieldsSize)
};

#endif  // UA_ENABLE_TYPEDESCRIPTION

/**
 * @}
 */

}  // namespace ua
}  // namespace opcua
