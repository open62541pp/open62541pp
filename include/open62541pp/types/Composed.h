#pragma once

#include <cstdint>

#include "open62541pp/Common.h"
#include "open62541pp/Config.h"
#include "open62541pp/NodeIds.h"  // ReferenceTypeId
#include "open62541pp/TypeConverter.h"
#include "open62541pp/TypeWrapper.h"
#include "open62541pp/types/Builtin.h"
#include "open62541pp/types/ExtensionObject.h"
#include "open62541pp/types/NodeId.h"

#include <initializer_list>
#include <string>
#include <vector>

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
#define UAPP_COMPOSED_GETTER_WRAPPER(WrapperType, getterName, member)                              \
    WrapperType& getterName() noexcept {                                                           \
        return asWrapper<WrapperType>(handle()->member);                                           \
    }                                                                                              \
    const WrapperType& getterName() const noexcept {                                               \
        return asWrapper<WrapperType>(handle()->member);                                           \
    }

// NOLINTNEXTLINE
#define UAPP_COMPOSED_GETTER_ARRAY(Type, getterName, array, size)                                  \
    size_t getterName##Size() const noexcept {                                                     \
        return handle()->size;                                                                     \
    }                                                                                              \
    std::vector<Type> getterName() const {                                                         \
        return detail::fromNativeArray<Type>(handle()->array, handle()->size);                     \
    }

namespace opcua {

/**
 * UA_ApplicationDescription wrapper class.
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
    UAPP_COMPOSED_GETTER_ARRAY(std::string, getDiscoveryUrls, discoveryUrls, discoveryUrlsSize)
};

/**
 * User identity token type.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.43
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
    UAPP_COMPOSED_GETTER_ARRAY(
        UserTokenPolicy, getUserIdentityTokens, userIdentityTokens, userIdentityTokensSize
    )
    UAPP_COMPOSED_GETTER_WRAPPER(String, getTransportProfileUri, transportProfileUri)
    UAPP_COMPOSED_GETTER(UA_Byte, getSecurityLevel, securityLevel)
};

/**
 * UA_UserIdentityToken wrapper class.
 * @ingroup TypeWrapper
 */
class UserIdentityToken : public TypeWrapper<UA_UserIdentityToken, UA_TYPES_USERIDENTITYTOKEN> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    UAPP_COMPOSED_GETTER_WRAPPER(String, getPolicyId, policyId)
};

/**
 * UA_AnonymousIdentityToken wrapper class.
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
        const NodeId& nodeId,
        BrowseDirection browseDirection,
        const NodeId& referenceType = ReferenceTypeId::References,
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

    UAPP_COMPOSED_GETTER(UA_StatusCode, getStatusCode, statusCode)
    UAPP_COMPOSED_GETTER_WRAPPER(ByteString, getContinuationPoint, continuationPoint)
    UAPP_COMPOSED_GETTER_ARRAY(ReferenceDescription, getReferences, references, referencesSize)
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
        const NodeId& referenceType,
        bool isInverse,
        bool includeSubtypes,
        const QualifiedName& targetName
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
    explicit RelativePath(const std::vector<RelativePathElement>& elements);

    UAPP_COMPOSED_GETTER_ARRAY(RelativePathElement, getElements, elements, elementsSize)
};

/**
 * UA_BrowsePath wrapper class.
 * @ingroup TypeWrapper
 */
class BrowsePath : public TypeWrapper<UA_BrowsePath, UA_TYPES_BROWSEPATH> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    BrowsePath(const NodeId& startingNode, const RelativePath& relativePath);

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

    UAPP_COMPOSED_GETTER(UA_StatusCode, getStatusCode, statusCode)
    UAPP_COMPOSED_GETTER_ARRAY(BrowsePathTarget, getTargets, targets, targetsSize)
};

/**
 * UA_ReadValueId wrapper class.
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.29
 * @ingroup TypeWrapper
 */
class ReadValueId : public TypeWrapper<UA_ReadValueId, UA_TYPES_READVALUEID> {
public:
    using TypeWrapperBase::TypeWrapperBase;

    ReadValueId(const NodeId& id, AttributeId attributeId);

    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getNodeId, nodeId)
    UAPP_COMPOSED_GETTER_CAST(AttributeId, getAttributeId, attributeId)
    UAPP_COMPOSED_GETTER_WRAPPER(String, getIndexRange, indexRange)
    UAPP_COMPOSED_GETTER_WRAPPER(QualifiedName, getDataEncoding, dataEncoding)
};

#ifdef UA_TYPES_ARGUMENT
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
        const LocalizedText& description,
        const NodeId& dataType,
        ValueRank valueRank = {},
        const std::vector<uint32_t>& arrayDimensions = {}
    );

    UAPP_COMPOSED_GETTER_WRAPPER(String, getName, name)
    UAPP_COMPOSED_GETTER_WRAPPER(LocalizedText, getDescription, description)
    UAPP_COMPOSED_GETTER_WRAPPER(NodeId, getDataType, dataType)
    UAPP_COMPOSED_GETTER_CAST(ValueRank, getValueRank, valueRank)
    UAPP_COMPOSED_GETTER_ARRAY(uint32_t, getArrayDimensions, arrayDimensions, arrayDimensionsSize)
};
#endif

#ifdef UA_TYPES_PERFORMUPDATETYPE
enum class PerformUpdateType : uint32_t {
    // clang-format off
    Insert  = 1,
    Replace = 2,
    Update  = 3,
    Remove  = 4,
    // clang-format on
};
#endif

}  // namespace opcua
