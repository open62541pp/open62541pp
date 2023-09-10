#pragma once

#include <cstdint>
#include <initializer_list>
#include <string_view>
#include <type_traits>
#include <utility>  // forward
#include <variant>

#include "open62541pp/Common.h"
#include "open62541pp/NodeIds.h"  // ReferenceTypeId
#include "open62541pp/Span.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/traits.h"
#include "open62541pp/open62541.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/DataValue.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/ExtensionObject.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/types/Variant.h"

// NOLINTNEXTLINE
#define UAPP_COMPOSED_GETTER(Type, getterName, member)                                             \
    Type getterName() const noexcept {                                                             \
        return handle()->member;                                                                   \
    }

// NOLINTNEXTLINE
#define UAPP_COMPOSED_GETTER_CAST(Type, getterName, member)                                        \
    Type getterName() const noexcept {                                                             \
        return static_cast<Type>(handle()->member);                                                \
    }

// NOLINTNEXTLINE
#define UAPP_COMPOSED_GETTER_WRAPPER_CONST(WrapperType, getterName, member)                        \
    const WrapperType& getterName() const noexcept {                                               \
        return asWrapper<WrapperType>(handle()->member);                                           \
    }
// NOLINTNEXTLINE
#define UAPP_COMPOSED_GETTER_WRAPPER_NONCONST(WrapperType, getterName, member)                     \
    WrapperType& getterName() noexcept {                                                           \
        return asWrapper<WrapperType>(handle()->member);                                           \
    }
// NOLINTNEXTLINE
#define UAPP_COMPOSED_GETTER_WRAPPER(WrapperType, getterName, member)                              \
    UAPP_COMPOSED_GETTER_WRAPPER_CONST(WrapperType, getterName, member)                            \
    UAPP_COMPOSED_GETTER_WRAPPER_NONCONST(WrapperType, getterName, member)

// NOLINTNEXTLINE
#define UAPP_COMPOSED_GETTER_SPAN(Type, getterName, memberArray, memberSize)                       \
    Span<const Type> getterName() const noexcept {                                                 \
        return {handle()->memberArray, handle()->memberSize};                                      \
    }                                                                                              \
    Span<Type> getterName() noexcept {                                                             \
        return {handle()->memberArray, handle()->memberSize};                                      \
    }
// NOLINTNEXTLINE
#define UAPP_COMPOSED_GETTER_SPAN_WRAPPER(Type, getterName, memberArray, memberSize)               \
    Span<const Type> getterName() const noexcept {                                                 \
        return {asWrapper<Type>(handle()->memberArray), handle()->memberSize};                     \
    }                                                                                              \
    Span<Type> getterName() noexcept {                                                             \
        return {asWrapper<Type>(handle()->memberArray), handle()->memberSize};                     \
    }

namespace opcua {

/**
 * UA_ApplicationDescription wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.2
 * @ingroup TypeWrapper
 */
class ApplicationDescription
    : public TypeWrapper<UA_ApplicationDescription, UA_TYPES_APPLICATIONDESCRIPTION> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(String, getApplicationUri, applicationUri)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getProductUri, productUri)
    UAPP_COMPOSED_GETTER_WRAPPER(LocalizedText, getApplicationName, applicationName)
    UAPP_COMPOSED_GETTER(UA_ApplicationType, getApplicationType, applicationType)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getGatewayServerUri, gatewayServerUri)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getDiscoveryProfileUri, discoveryProfileUri)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(String, getDiscoveryUrls, discoveryUrls, discoveryUrlsSize)
};

/**
 * UA_RequestHeader wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.33
 * @ingroup TypeWrapper
 */
class RequestHeader : public TypeWrapper<UA_RequestHeader, UA_TYPES_REQUESTHEADER> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    RequestHeader(
        NodeId authenticationToken,
        DateTime timestamp,
        uint32_t requestHandle,
        uint32_t returnDiagnostics,
        std::string_view auditEntryId,
        uint32_t timeoutHint,
        ExtensionObject additionalHeader
    );

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getAuthenticationToken, authenticationToken)
    UAPP_COMPOSED_GETTER_WRAPPER(DateTime, getTimestamp, timestamp)
    UAPP_COMPOSED_GETTER(uint32_t, getRequestHandle, requestHandle)
    UAPP_COMPOSED_GETTER(uint32_t, getReturnDiagnostics, returnDiagnostics)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getAuditEntryId, auditEntryId)
    UAPP_COMPOSED_GETTER(uint32_t, getTimeoutHint, timeoutHint)
    UAPP_COMPOSED_GETTER_WRAPPER(ExtensionObject, getAdditionalHeader, additionalHeader)
};

/**
 * UA_ResponseHeader wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.34
 * @ingroup TypeWrapper
 */
class ResponseHeader : public TypeWrapper<UA_ResponseHeader, UA_TYPES_RESPONSEHEADER> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(DateTime, getTimestamp, timestamp)
    UAPP_COMPOSED_GETTER(uint32_t, getRequestHandle, requestHandle)
    UAPP_COMPOSED_GETTER(StatusCode, getServiceResult, serviceResult)
    UAPP_COMPOSED_GETTER_WRAPPER(DiagnosticInfo, getServiceDiagnostics, serviceDiagnostics)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(String, getStringTable, stringTable, stringTableSize)
    UAPP_COMPOSED_GETTER_WRAPPER(ExtensionObject, getAdditionalHeader, additionalHeader)
};

/**
 * User identity token type.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.43
 * @ingroup TypeWrapper
 */
enum class UserTokenType : uint32_t {
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
 * @ingroup TypeWrapper
 */
class UserTokenPolicy : public TypeWrapper<UA_UserTokenPolicy, UA_TYPES_USERTOKENPOLICY> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UserTokenPolicy(
        std::string_view policyId,
        UserTokenType tokenType,
        std::string_view issuedTokenType = {},
        std::string_view issuerEndpointUrl = {},
        std::string_view securityPolicyUri = {}
    );

    UAPP_COMPOSED_GETTER_WRAPPER(String, getPolicyId, policyId)
    UAPP_COMPOSED_GETTER_CAST(UserTokenType, getTokenType, tokenType)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getIssuedTokenType, issuedTokenType)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getIssuerEndpointUrl, issuerEndpointUrl)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getSecurityPolicyUri, securityPolicyUri)
};

/**
 * UA_EndpointDescription wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.14
 * @ingroup TypeWrapper
 */
class EndpointDescription
    : public TypeWrapper<UA_EndpointDescription, UA_TYPES_ENDPOINTDESCRIPTION> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(String, getEndpointUrl, endpointUrl)
    UAPP_COMPOSED_GETTER_WRAPPER(ApplicationDescription, getServer, server)
    UAPP_COMPOSED_GETTER_WRAPPER(ByteString, getServerCertificate, serverCertificate)
    UAPP_COMPOSED_GETTER(UA_MessageSecurityMode, getSecurityMode, securityMode)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getSecurityPolicyUri, securityPolicyUri)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(
        UserTokenPolicy, getUserIdentityTokens, userIdentityTokens, userIdentityTokensSize
    )
    UAPP_COMPOSED_GETTER_WRAPPER(String, getTransportProfileUri, transportProfileUri)
    UAPP_COMPOSED_GETTER(UA_Byte, getSecurityLevel, securityLevel)
};

/* --------------------------------------- Node attributes -------------------------------------- */

// Specifialized macros to generate getters/setters for `UA_*Attribute` classes.
// The `specifiedAttributes` mask is automatically updated in the setter methods.
// A fluent interface is used for the setter methods.

// NOLINTNEXTLINE
#define UAPP_NODEATTR(Type, suffix, member, flag)                                                  \
    UAPP_COMPOSED_GETTER(Type, get##suffix, member)                                                \
    auto& set##suffix(Type member) noexcept {                                                      \
        handle()->specifiedAttributes |= flag;                                                     \
        handle()->member = member;                                                                 \
        return *this;                                                                              \
    }

// NOLINTNEXTLINE
#define UAPP_NODEATTR_CAST(Type, suffix, member, flag)                                             \
    UAPP_COMPOSED_GETTER_CAST(Type, get##suffix, member)                                           \
    auto& set##suffix(Type member) noexcept {                                                      \
        handle()->specifiedAttributes |= flag;                                                     \
        handle()->member = static_cast<decltype(handle()->member)>(member);                        \
        return *this;                                                                              \
    }

// NOLINTNEXTLINE
#define UAPP_NODEATTR_WRAPPER(WrapperType, suffix, member, flag)                                   \
    UAPP_COMPOSED_GETTER_WRAPPER_CONST(WrapperType, get##suffix, member)                           \
    auto& set##suffix(const WrapperType& member) {                                                 \
        handle()->specifiedAttributes |= flag;                                                     \
        asWrapper<WrapperType>(handle()->member) = member;                                         \
        return *this;                                                                              \
    }

// NOLINTNEXTLINE
#define UAPP_NODEATTR_ARRAY(Type, suffix, memberArray, memberSize, flag)                           \
    UAPP_COMPOSED_GETTER_SPAN(Type, get##suffix, memberArray, memberSize)                          \
    auto& set##suffix(Span<const Type> memberArray) {                                              \
        handle()->specifiedAttributes |= flag;                                                     \
        UA_Array_delete(                                                                           \
            handle()->memberArray, handle()->memberSize, &detail::guessDataType<Type>()            \
        );                                                                                         \
        handle()->memberArray = detail::toNativeArrayAlloc(                                        \
            memberArray.begin(), memberArray.end()                                                 \
        );                                                                                         \
        handle()->memberSize = memberArray.size();                                                 \
        return *this;                                                                              \
    }

// NOLINTNEXTLINT
#define UAPP_NODEATTR_COMMON                                                                       \
    UAPP_COMPOSED_GETTER(uint32_t, getSpecifiedAttributes, specifiedAttributes)                    \
    UAPP_NODEATTR_WRAPPER(                                                                         \
        LocalizedText, DisplayName, displayName, UA_NODEATTRIBUTESMASK_DISPLAYNAME                 \
    )                                                                                              \
    UAPP_NODEATTR_WRAPPER(                                                                         \
        LocalizedText, Description, description, UA_NODEATTRIBUTESMASK_DESCRIPTION                 \
    )                                                                                              \
    UAPP_NODEATTR(uint32_t, WriteMask, writeMask, UA_NODEATTRIBUTESMASK_WRITEMASK)                 \
    UAPP_NODEATTR(uint32_t, UserWriteMask, userWriteMask, UA_NODEATTRIBUTESMASK_USERWRITEMASK)

/**
 * UA_NodeAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24
 * @ingroup TypeWrapper
 */
class NodeAttributes : public TypeWrapper<UA_NodeAttributes, UA_TYPES_NODEATTRIBUTES> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_NODEATTR_COMMON
};

/**
 * UA_ObjectAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.2
 * @ingroup TypeWrapper
 */
class ObjectAttributes : public TypeWrapper<UA_ObjectAttributes, UA_TYPES_OBJECTTYPEATTRIBUTES> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    /// Construct with default attribute definitions.
    ObjectAttributes();

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(uint8_t, EventNotifier, eventNotifier, UA_NODEATTRIBUTESMASK_EVENTNOTIFIER)
};

/**
 * UA_VariableAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.3
 * @ingroup TypeWrapper
 */
class VariableAttributes : public TypeWrapper<UA_VariableAttributes, UA_TYPES_VARIABLEATTRIBUTES> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    /// Construct with default attribute definitions.
    VariableAttributes();

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
        return setDataType(asWrapper<NodeId>(detail::guessDataType<T>().typeId));
    }

    UAPP_NODEATTR_CAST(ValueRank, ValueRank, valueRank, UA_NODEATTRIBUTESMASK_VALUERANK)
    UAPP_NODEATTR_ARRAY(
        uint32_t,
        ArrayDimensions,
        arrayDimensions,
        arrayDimensionsSize,
        UA_NODEATTRIBUTESMASK_ARRAYDIMENSIONS
    )
    UAPP_NODEATTR(uint8_t, AccessLevel, accessLevel, UA_NODEATTRIBUTESMASK_ACCESSLEVEL)
    UAPP_NODEATTR(uint8_t, UserAccessLevel, userAccessLevel, UA_NODEATTRIBUTESMASK_USERACCESSLEVEL)
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
 * @ingroup TypeWrapper
 */
class MethodAttributes : public TypeWrapper<UA_MethodAttributes, UA_TYPES_METHODATTRIBUTES> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    /// Construct with default attribute definitions.
    MethodAttributes();

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, Executable, executable, UA_NODEATTRIBUTESMASK_EXECUTABLE)
    UAPP_NODEATTR(bool, UserExecutable, userExecutable, UA_NODEATTRIBUTESMASK_USEREXECUTABLE)
};

/**
 * UA_ObjectTypeAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.5
 * @ingroup TypeWrapper
 */
class ObjectTypeAttributes
    : public TypeWrapper<UA_ObjectTypeAttributes, UA_TYPES_OBJECTTYPEATTRIBUTES> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    /// Construct with default attribute definitions.
    ObjectTypeAttributes();

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, IsAbstract, isAbstract, UA_NODEATTRIBUTESMASK_ISABSTRACT)
};

/**
 * UA_VariableAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.6
 * @ingroup TypeWrapper
 */
class VariableTypeAttributes
    : public TypeWrapper<UA_VariableTypeAttributes, UA_TYPES_VARIABLETYPEATTRIBUTES> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    /// Construct with default attribute definitions.
    VariableTypeAttributes();

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
        return setDataType(asWrapper<NodeId>(detail::guessDataType<T>().typeId));
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
 * @ingroup TypeWrapper
 */
class ReferenceTypeAttributes
    : public TypeWrapper<UA_ReferenceTypeAttributes, UA_TYPES_REFERENCETYPEATTRIBUTES> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    /// Construct with default attribute definitions.
    ReferenceTypeAttributes();

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
 * @ingroup TypeWrapper
 */
class DataTypeAttributes : public TypeWrapper<UA_DataTypeAttributes, UA_TYPES_DATATYPEATTRIBUTES> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    /// Construct with default attribute definitions.
    DataTypeAttributes();

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, IsAbstract, isAbstract, UA_NODEATTRIBUTESMASK_ISABSTRACT)
};

/**
 * UA_ViewAttributes wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.24.9
 * @ingroup TypeWrapper
 */
class ViewAttributes : public TypeWrapper<UA_ViewAttributes, UA_TYPES_VIEWATTRIBUTES> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    /// Construct with default attribute definitions.
    ViewAttributes();

    UAPP_NODEATTR_COMMON
    UAPP_NODEATTR(bool, IsAbstract, containsNoLoops, UA_NODEATTRIBUTESMASK_CONTAINSNOLOOPS)
    UAPP_NODEATTR(uint8_t, EventNotifier, eventNotifier, UA_NODEATTRIBUTESMASK_EVENTNOTIFIER)
};

#undef UAPP_NODEATTR
#undef UAPP_NODEATTR_WRAPPER
#undef UAPP_NODEATTR_ARRAY
#undef UAPP_NODEATTR_COMMON

/* ------------------------------------------- Browse ------------------------------------------- */

/**
 * UA_UserIdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41
 * @ingroup TypeWrapper
 */
class UserIdentityToken : public TypeWrapper<UA_UserIdentityToken, UA_TYPES_USERIDENTITYTOKEN> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(String, getPolicyId, policyId)
};

/**
 * UA_AnonymousIdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41.3
 * @ingroup TypeWrapper
 */
class AnonymousIdentityToken
    : public TypeWrapper<UA_AnonymousIdentityToken, UA_TYPES_ANONYMOUSIDENTITYTOKEN> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(String, getPolicyId, policyId)
};

/**
 * UA_UserNameIdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41.4
 * @ingroup TypeWrapper
 */
class UserNameIdentityToken
    : public TypeWrapper<UA_UserNameIdentityToken, UA_TYPES_USERNAMEIDENTITYTOKEN> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(String, getPolicyId, policyId)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getUserName, userName)
    UAPP_COMPOSED_GETTER_WRAPPER(ByteString, getPassword, password)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getEncryptionAlgorithm, encryptionAlgorithm)
};

/**
 * UA_X509IdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41.5
 * @ingroup TypeWrapper
 */
class X509IdentityToken : public TypeWrapper<UA_X509IdentityToken, UA_TYPES_X509IDENTITYTOKEN> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(String, getPolicyId, policyId)
    UAPP_COMPOSED_GETTER_WRAPPER(ByteString, getCertificateData, certificateData)
};

/**
 * UA_IssuedIdentityToken wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.41.6
 * @ingroup TypeWrapper
 */
class IssuedIdentityToken
    : public TypeWrapper<UA_IssuedIdentityToken, UA_TYPES_ISSUEDIDENTITYTOKEN> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(String, getPolicyId, policyId)
    UAPP_COMPOSED_GETTER_WRAPPER(ByteString, getTokenData, tokenData)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getEncryptionAlgorithm, encryptionAlgorithm)
};

/**
 * UA_AddNodesItem wrapper class.
 * @ingroup TypeWrapper
 */
class AddNodesItem : public TypeWrapper<UA_AddNodesItem, UA_TYPES_ADDNODESITEM> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(ExpandedNodeId, getParentNodeId, parentNodeId)
    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getReferenceTypeId, referenceTypeId)
    UAPP_COMPOSED_GETTER_WRAPPER(ExpandedNodeId, getRequestedNewNodeId, requestedNewNodeId)
    UAPP_COMPOSED_GETTER_WRAPPER(QualifiedName, getBrowseName, browseName)
    UAPP_COMPOSED_GETTER_CAST(NodeClass, getNodeClass, nodeClass)
    UAPP_COMPOSED_GETTER_WRAPPER(ExtensionObject, getNodeAttributes, nodeAttributes)
    UAPP_COMPOSED_GETTER_WRAPPER(ExpandedNodeId, getTypeDefinition, typeDefinition)
};

/**
 * UA_AddReferencesItem wrapper class.
 * @ingroup TypeWrapper
 */
class AddReferencesItem : public TypeWrapper<UA_AddReferencesItem, UA_TYPES_ADDREFERENCESITEM> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getSourceNodeId, sourceNodeId)
    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getReferenceTypeId, referenceTypeId)
    UAPP_COMPOSED_GETTER(bool, getIsForward, isForward)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getTargetServerUri, targetServerUri)
    UAPP_COMPOSED_GETTER_WRAPPER(ExpandedNodeId, getTargetNodeId, targetNodeId)
    UAPP_COMPOSED_GETTER_CAST(NodeClass, getTargetNodeClass, targetNodeClass)
};

/**
 * UA_DeleteNodesItem wrapper class.
 * @ingroup TypeWrapper
 */
class DeleteNodesItem : public TypeWrapper<UA_DeleteNodesItem, UA_TYPES_DELETENODESITEM> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getNodeId, nodeId)
    UAPP_COMPOSED_GETTER(bool, getDeleteTargetReferences, deleteTargetReferences)
};

/**
 * UA_DeleteReferencesItem wrapper class.
 * @ingroup TypeWrapper
 */
class DeleteReferencesItem
    : public TypeWrapper<UA_DeleteReferencesItem, UA_TYPES_DELETEREFERENCESITEM> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getSourceNodeId, sourceNodeId)
    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getReferenceTypeId, referenceTypeId)
    UAPP_COMPOSED_GETTER(bool, getIsForward, isForward)
    UAPP_COMPOSED_GETTER_WRAPPER(ExpandedNodeId, getTargetNodeId, targetNodeId)
    UAPP_COMPOSED_GETTER(bool, getDeleteBidirectional, deleteBidirectional)
};

/**
 * UA_BrowseDescription wrapper class.
 * @ingroup TypeWrapper
 */
class BrowseDescription : public TypeWrapper<UA_BrowseDescription, UA_TYPES_BROWSEDESCRIPTION> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    BrowseDescription(
        NodeId nodeId,
        BrowseDirection browseDirection,
        NodeId referenceType = ReferenceTypeId::References,
        bool includeSubtypes = true,
        uint32_t nodeClassMask = UA_NODECLASS_UNSPECIFIED,
        uint32_t resultMask = UA_BROWSERESULTMASK_ALL
    );

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getNodeId, nodeId)
    UAPP_COMPOSED_GETTER_CAST(BrowseDirection, getBrowseDirection, browseDirection)
    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getReferenceTypeId, referenceTypeId)
    UAPP_COMPOSED_GETTER(bool, getIncludeSubtypes, includeSubtypes)
    UAPP_COMPOSED_GETTER(uint32_t, getNodeClassMask, nodeClassMask)
    UAPP_COMPOSED_GETTER(uint32_t, getResultMask, resultMask)
};

/**
 * UA_ReferenceDescription wrapper class.
 * @ingroup TypeWrapper
 */
class ReferenceDescription
    : public TypeWrapper<UA_ReferenceDescription, UA_TYPES_REFERENCEDESCRIPTION> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getReferenceTypeId, referenceTypeId)
    UAPP_COMPOSED_GETTER(bool, getIsForward, isForward)
    UAPP_COMPOSED_GETTER_WRAPPER(ExpandedNodeId, getNodeId, nodeId)
    UAPP_COMPOSED_GETTER_WRAPPER(QualifiedName, getBrowseName, browseName)
    UAPP_COMPOSED_GETTER_WRAPPER(LocalizedText, getDisplayName, displayName)
    UAPP_COMPOSED_GETTER_CAST(NodeClass, getNodeClass, nodeClass)
    UAPP_COMPOSED_GETTER_WRAPPER(ExpandedNodeId, getTypeDefinition, typeDefinition)
};

/**
 * UA_BrowseResult wrapper class.
 * @ingroup TypeWrapper
 */
class BrowseResult : public TypeWrapper<UA_BrowseResult, UA_TYPES_BROWSERESULT> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER(StatusCode, getStatusCode, statusCode)
    UAPP_COMPOSED_GETTER_WRAPPER(ByteString, getContinuationPoint, continuationPoint)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(
        ReferenceDescription, getReferences, references, referencesSize
    )
};

/**
 * UA_RelativePathElement wrapper class.
 * @ingroup TypeWrapper
 */
class RelativePathElement
    : public TypeWrapper<UA_RelativePathElement, UA_TYPES_RELATIVEPATHELEMENT> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    RelativePathElement(
        NodeId referenceType, bool isInverse, bool includeSubtypes, QualifiedName targetName
    );

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getReferenceTypeId, referenceTypeId)
    UAPP_COMPOSED_GETTER(bool, getIsInverse, isInverse)
    UAPP_COMPOSED_GETTER(bool, getIncludeSubtypes, includeSubtypes)
    UAPP_COMPOSED_GETTER_WRAPPER(QualifiedName, getTargetName, targetName)
};

/**
 * UA_RelativePath wrapper class.
 * @ingroup TypeWrapper
 */
class RelativePath : public TypeWrapper<UA_RelativePath, UA_TYPES_RELATIVEPATH> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    RelativePath(std::initializer_list<RelativePathElement> elements);
    explicit RelativePath(Span<const RelativePathElement> elements);

    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(RelativePathElement, getElements, elements, elementsSize)
};

/**
 * UA_BrowsePath wrapper class.
 * @ingroup TypeWrapper
 */
class BrowsePath : public TypeWrapper<UA_BrowsePath, UA_TYPES_BROWSEPATH> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    BrowsePath(NodeId startingNode, RelativePath relativePath);

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getStartingNode, startingNode)
    UAPP_COMPOSED_GETTER_WRAPPER(RelativePath, getRelativePath, relativePath)
};

/**
 * UA_BrowsePathTarget wrapper class.
 * @ingroup TypeWrapper
 */
class BrowsePathTarget : public TypeWrapper<UA_BrowsePathTarget, UA_TYPES_BROWSEPATHTARGET> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(ExpandedNodeId, getTargetId, targetId)
    UAPP_COMPOSED_GETTER(uint32_t, getRemainingPathIndex, remainingPathIndex)
};

/**
 * UA_BrowsePathResult wrapper class.
 * @ingroup TypeWrapper
 */
class BrowsePathResult : public TypeWrapper<UA_BrowsePathResult, UA_TYPES_BROWSEPATHRESULT> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER(StatusCode, getStatusCode, statusCode)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(BrowsePathTarget, getTargets, targets, targetsSize)
};

/**
 * UA_ReadValueId wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.29
 * @ingroup TypeWrapper
 */
class ReadValueId : public TypeWrapper<UA_ReadValueId, UA_TYPES_READVALUEID> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    ReadValueId(
        NodeId nodeId,
        AttributeId attributeId,
        std::string_view indexRange = {},
        QualifiedName dataEncoding = {}
    );

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getNodeId, nodeId)
    UAPP_COMPOSED_GETTER_CAST(AttributeId, getAttributeId, attributeId)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getIndexRange, indexRange)
    UAPP_COMPOSED_GETTER_WRAPPER(QualifiedName, getDataEncoding, dataEncoding)
};

/**
 * UA_ReadRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.2
 * @ingroup TypeWrapper
 */
class ReadRequest : public TypeWrapper<UA_ReadRequest, UA_TYPES_READREQUEST> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    ReadRequest(
        RequestHeader requestHeader,
        double maxAge,
        TimestampsToReturn timestampsToReturn,
        Span<const ReadValueId> nodesToRead
    );

    UAPP_COMPOSED_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_COMPOSED_GETTER(double, getMaxAge, maxAge)
    UAPP_COMPOSED_GETTER_CAST(TimestampsToReturn, getTimestampsToReturn, timestampsToReturn)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(ReadValueId, getNodesToRead, nodesToRead, nodesToReadSize)
};

/**
 * UA_ReadResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.2
 * @ingroup TypeWrapper
 */
class ReadResponse : public TypeWrapper<UA_ReadResponse, UA_TYPES_READRESPONSE> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(DataValue, getResults, results, resultsSize)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/**
 * UA_WriteValue wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.4
 * @ingroup TypeWrapper
 */
class WriteValue : public TypeWrapper<UA_WriteValue, UA_TYPES_WRITEVALUE> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    WriteValue(
        NodeId nodeId, AttributeId attributeId, std::string_view indexRange, DataValue value
    );

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getNodeId, nodeId)
    UAPP_COMPOSED_GETTER_CAST(AttributeId, getAttributeId, attributeId)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getIndexRange, indexRange)
    UAPP_COMPOSED_GETTER_WRAPPER(DataValue, getValue, value)
};

/**
 * UA_WriteRequest wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.4
 * @ingroup TypeWrapper
 */
class WriteRequest : public TypeWrapper<UA_WriteRequest, UA_TYPES_WRITEREQUEST> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    WriteRequest(RequestHeader requestHeader, Span<const WriteValue> nodesToWrite);

    UAPP_COMPOSED_GETTER_WRAPPER(RequestHeader, getRequestHeader, requestHeader)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(WriteValue, getNodesToWrite, nodesToWrite, nodesToWriteSize)
};

/**
 * UA_WriteResponse wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.10.4
 * @ingroup TypeWrapper
 */
class WriteResponse : public TypeWrapper<UA_WriteResponse, UA_TYPES_WRITERESPONSE> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(ResponseHeader, getResponseHeader, responseHeader)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(StatusCode, getResults, results, resultsSize)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(
        DiagnosticInfo, getDiagnosticInfos, diagnosticInfos, diagnosticInfosSize
    )
};

/* ------------------------------------------- Method ------------------------------------------- */

#ifdef UA_ENABLE_METHODCALLS

/**
 * UA_Argument wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.6
 * @ingroup TypeWrapper
 */
class Argument : public TypeWrapper<UA_Argument, UA_TYPES_ARGUMENT> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    Argument(
        std::string_view name,
        LocalizedText description,
        NodeId dataType,
        ValueRank valueRank = {},
        Span<const uint32_t> arrayDimensions = {}
    );

    UAPP_COMPOSED_GETTER_WRAPPER(String, getName, name)
    UAPP_COMPOSED_GETTER_WRAPPER(LocalizedText, getDescription, description)
    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getDataType, dataType)
    UAPP_COMPOSED_GETTER_CAST(ValueRank, getValueRank, valueRank)
    UAPP_COMPOSED_GETTER_SPAN(uint32_t, getArrayDimensions, arrayDimensions, arrayDimensionsSize)
};

#endif

/* ---------------------------------------- Subscriptions --------------------------------------- */

#ifdef UA_ENABLE_SUBSCRIPTIONS

/**
 * Filter operator.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.3
 * @ingroup TypeWrapper
 */
enum class FilterOperator : uint32_t {
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
 * @ingroup TypeWrapper
 */
class ElementOperand : public TypeWrapper<UA_ElementOperand, UA_TYPES_ELEMENTOPERAND> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    explicit ElementOperand(uint32_t index);

    UAPP_COMPOSED_GETTER(uint32_t, getIndex, index)
};

/**
 * UA_LiteralOperand wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.4.3
 * @ingroup TypeWrapper
 */
class LiteralOperand : public TypeWrapper<UA_LiteralOperand, UA_TYPES_LITERALOPERAND> {
private:
    template <typename T>
    using EnableIfLiteral =
        std::enable_if_t<!detail::IsOneOf<T, Variant, UA_LiteralOperand, LiteralOperand>::value>;

public:
    using TypeWrapperBase::TypeWrapperBase;

    explicit LiteralOperand(Variant value);

    template <typename T, typename = EnableIfLiteral<T>>
    explicit LiteralOperand(T&& literal)
        : LiteralOperand(Variant::fromScalar(std::forward<T>(literal))) {}

    UAPP_COMPOSED_GETTER_WRAPPER(Variant, getValue, value)
};

/**
 * UA_AttributeOperand wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.4.4
 * @ingroup TypeWrapper
 */
class AttributeOperand : public TypeWrapper<UA_AttributeOperand, UA_TYPES_ATTRIBUTEOPERAND> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    AttributeOperand(
        NodeId nodeId,
        std::string_view alias,
        RelativePath browsePath,
        AttributeId attributeId,
        std::string_view indexRange = {}
    );

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getNodeId, nodeId)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getAlias, alias)
    UAPP_COMPOSED_GETTER_WRAPPER(RelativePath, getBrowsePath, browsePath)
    UAPP_COMPOSED_GETTER_CAST(AttributeId, getAttributeId, attributeId)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getIndexRange, indexRange)
};

/**
 * UA_SimpleAttributeOperand wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.7.4.5
 * @ingroup TypeWrapper
 */
class SimpleAttributeOperand
    : public TypeWrapper<UA_SimpleAttributeOperand, UA_TYPES_SIMPLEATTRIBUTEOPERAND> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    SimpleAttributeOperand(
        NodeId typeDefinitionId,
        Span<const QualifiedName> browsePath,
        AttributeId attributeId,
        std::string_view indexRange = {}
    );

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getTypeDefinitionId, typeDefinitionId)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(QualifiedName, getBrowsePath, browsePath, browsePathSize)
    UAPP_COMPOSED_GETTER_CAST(AttributeId, getAttributeId, attributeId)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getIndexRange, indexRange)
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
 * @ingroup TypeWrapper
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
 * @ingroup TypeWrapper
 */
class ContentFilterElement
    : public TypeWrapper<UA_ContentFilterElement, UA_TYPES_CONTENTFILTERELEMENT> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    ContentFilterElement(FilterOperator filterOperator, Span<const FilterOperand> operands);

    UAPP_COMPOSED_GETTER_CAST(FilterOperator, getFilterOperator, filterOperator)
    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(
        ExtensionObject, getFilterOperands, filterOperands, filterOperandsSize
    )
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
 * @ingroup TypeWrapper
 */
class ContentFilter : public TypeWrapper<UA_ContentFilter, UA_TYPES_CONTENTFILTER> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    ContentFilter(std::initializer_list<ContentFilterElement> elements);
    explicit ContentFilter(Span<const ContentFilterElement> elements);

    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(ContentFilterElement, getElements, elements, elementsSize)
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
 * @ingroup TypeWrapper
 */
enum class DataChangeTrigger : uint32_t {
    // clang-format off
    Status               = 0,
    StatusValue          = 1,
    StatusValueTimestamp = 2,
    // clang-format on
};

/**
 * Deadband type.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.22.2
 * @ingroup TypeWrapper
 */
enum class DeadbandType : uint32_t {
    // clang-format off
    None     = 0,
    Absolute = 1,
    Percent  = 2,
    // clang-format on
};

/**
 * UA_DataChangeFilter wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.22.2
 * @ingroup TypeWrapper
 */
class DataChangeFilter : public TypeWrapper<UA_DataChangeFilter, UA_TYPES_DATACHANGEFILTER> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    DataChangeFilter(DataChangeTrigger trigger, DeadbandType deadbandType, double deadbandValue);

    UAPP_COMPOSED_GETTER_CAST(DataChangeTrigger, getTrigger, trigger)
    UAPP_COMPOSED_GETTER_CAST(DeadbandType, getDeadbandType, deadbandType)
    UAPP_COMPOSED_GETTER(double, getDeadbandValue, deadbandValue)
};

/**
 * UA_EventFilter wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.22.3
 * @ingroup TypeWrapper
 */
class EventFilter : public TypeWrapper<UA_EventFilter, UA_TYPES_EVENTFILTER> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    EventFilter(Span<const SimpleAttributeOperand> selectClauses, ContentFilter whereClause);

    UAPP_COMPOSED_GETTER_SPAN_WRAPPER(
        SimpleAttributeOperand, getSelectClauses, selectClauses, selectClausesSize
    )
    UAPP_COMPOSED_GETTER_WRAPPER(ContentFilter, getWhereClause, whereClause)
};

using AggregateConfiguration = UA_AggregateConfiguration;

/**
 * UA_AggregateFilter wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.22.4
 * @ingroup TypeWrapper
 */
class AggregateFilter : public TypeWrapper<UA_AggregateFilter, UA_TYPES_AGGREGATEFILTER> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    AggregateFilter(
        DateTime startTime,
        NodeId aggregateType,
        double processingInterval,
        AggregateConfiguration aggregateConfiguration
    );

    UAPP_COMPOSED_GETTER_WRAPPER(DateTime, getStartTime, startTime)
    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getAggregateType, aggregateType)
    UAPP_COMPOSED_GETTER(double, getProcessingInterval, processingInterval)
    UAPP_COMPOSED_GETTER(AggregateConfiguration, getAggregateConfiguration, aggregateConfiguration)
};

#endif

/* ----------------------------------------- Historizing ---------------------------------------- */

/**
 * Perform update type for structured data history updates.
 * @see UA_PerformUpdateType
 * @see https://reference.opcfoundation.org/Core/Part11/v104/docs/6.8.3
 */
enum class PerformUpdateType : uint32_t {
    // clang-format off
    Insert  = 1,
    Replace = 2,
    Update  = 3,
    Remove  = 4,
    // clang-format on
};

}  // namespace opcua
