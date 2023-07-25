#pragma once

#include <cstdint>
#include <string_view>
#include <tuple>

#include "open62541pp/detail/traits.h"
#include "open62541pp/open62541.h"

namespace opcua {

/// Type index of the ::UA_TYPES array.
using TypeIndex = uint16_t;

/**
 * Built-in types.
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.1.2
 */
enum class Type : TypeIndex {
    // clang-format off
    Boolean         = UA_TYPES_BOOLEAN,
    SByte           = UA_TYPES_SBYTE,
    Byte            = UA_TYPES_BYTE,
    Int16           = UA_TYPES_INT16,
    UInt16          = UA_TYPES_UINT16,
    Int32           = UA_TYPES_INT32,
    UInt32          = UA_TYPES_UINT32,
    Int64           = UA_TYPES_INT64,
    UInt64          = UA_TYPES_UINT64,
    Float           = UA_TYPES_FLOAT,
    Double          = UA_TYPES_DOUBLE,
    String          = UA_TYPES_STRING,
    DateTime        = UA_TYPES_DATETIME,
    Guid            = UA_TYPES_GUID,
    ByteString      = UA_TYPES_BYTESTRING,
    XmlElement      = UA_TYPES_XMLELEMENT,
    NodeId          = UA_TYPES_NODEID,
    ExpandedNodeId  = UA_TYPES_EXPANDEDNODEID,
    StatusCode      = UA_TYPES_STATUSCODE,
    QualifiedName   = UA_TYPES_QUALIFIEDNAME,
    LocalizedText   = UA_TYPES_LOCALIZEDTEXT,
    ExtensionObject = UA_TYPES_EXTENSIONOBJECT,
    DataValue       = UA_TYPES_DATAVALUE,
    Variant         = UA_TYPES_VARIANT,
    DiagnosticInfo  = UA_TYPES_DIAGNOSTICINFO
    // clang-format on
};

/**
 * Attribute identifiers.
 * @see UA_AttributeId
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/A.1/
 */
enum class AttributeId : uint32_t {
    // clang-format off
    NodeId                  = 1,
    NodeClass               = 2,
    BrowseName              = 3,
    DisplayName             = 4,
    Description             = 5,
    WriteMask               = 6,
    UserWriteMask           = 7,
    IsAbstract              = 8,
    Symmetric               = 9,
    InverseName             = 10,
    ContainsNoLoops         = 11,
    EventNotifier           = 12,
    Value                   = 13,
    DataType                = 14,
    ValueRank               = 15,
    ArrayDimensions         = 16,
    AccessLevel             = 17,
    UserAccessLevel         = 18,
    MinimumSamplingInterval = 19,
    Historizing             = 20,
    Executable              = 21,
    UserExecutable          = 22,
    DataTypeDefinition      = 23,
    RolePermissions         = 24,
    UserRolePermissions     = 25,
    AccessRestrictions      = 26,
    AccessLevelEx           = 27,
    // clang-format on
};

/**
 * Node classes.
 * @see UA_NodeClass
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/8.29
 */
enum class NodeClass : uint32_t {
    // clang-format off
    Unspecified   = 0,
    Object        = 1,
    Variable      = 2,
    Method        = 4,
    ObjectType    = 8,
    VariableType  = 16,
    ReferenceType = 32,
    DataType      = 64,
    View          = 128,
    // clang-format on
};

/// Get name of node class.
constexpr std::string_view getNodeClassName(NodeClass nodeClass) {
    switch (nodeClass) {
    case NodeClass::Object:
        return "Object";
    case NodeClass::Variable:
        return "Variable";
    case NodeClass::Method:
        return "Method";
    case NodeClass::ObjectType:
        return "ObjectType";
    case NodeClass::VariableType:
        return "VariableType";
    case NodeClass::ReferenceType:
        return "ReferenceType";
    case NodeClass::DataType:
        return "DataType";
    case NodeClass::View:
        return "View";
    default:
        return "Unknown";
    }
}

/**
 * Value rank.
 */
enum class ValueRank : int32_t {
    // clang-format off
    ScalarOrOneDimension = UA_VALUERANK_SCALAR_OR_ONE_DIMENSION,
    Any                  = UA_VALUERANK_ANY,
    Scalar               = UA_VALUERANK_SCALAR,
    OneOrMoreDimensions  = UA_VALUERANK_ONE_OR_MORE_DIMENSIONS,
    OneDimension         = UA_VALUERANK_ONE_DIMENSION,
    TwoDimensions        = UA_VALUERANK_TWO_DIMENSIONS,
    ThreeDimensions      = UA_VALUERANK_THREE_DIMENSIONS,
    // clang-format on
};

/**
 * Modelling rules.
 * @see https://reference.opcfoundation.org/Core/Part3/v105/docs/6.4.4
 */
enum class ModellingRule : uint16_t {
    // clang-format off
    Mandatory            = UA_NS0ID_MODELLINGRULE_MANDATORY,
    Optional             = UA_NS0ID_MODELLINGRULE_OPTIONAL,
    ExposesItsArray      = UA_NS0ID_MODELLINGRULE_EXPOSESITSARRAY,
    OptionalPlaceholder  = UA_NS0ID_MODELLINGRULE_OPTIONALPLACEHOLDER,
    MandatoryPlaceholder = UA_NS0ID_MODELLINGRULE_MANDATORYPLACEHOLDER,
    // clang-format on
};

/**
 * Browse direction.
 * An enumeration that specifies the direction of references to follow.
 * @see UA_BrowseDirection
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.5
 */
enum class BrowseDirection : uint32_t {
    // clang-format off
    Forward = 0,
    Inverse = 1,
    Both    = 2,
    Invalid = 3,
    // clang-format on
};

/**
 * Timestamps to return.
 * @see UA_TimestampsToReturn
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.40
 */
enum class TimestampsToReturn : uint32_t {
    // clang-format off
    Source   = 0,
    Server   = 1,
    Both     = 2,
    Neither  = 3,
    Invalid  = 4,
    // clang-format on
};

/**
 * Monitoring mode.
 * @see UA_MonitoringMode
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.23
 */
enum class MonitoringMode : uint32_t {
    // clang-format off
    Disabled  = 0,
    Sampling  = 1,
    Reporting = 2,
    // clang-format on
};

/**
 * Message security mode.
 * @see UA_MessageSecurityMode
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/7.20
 */
enum class MessageSecurityMode : uint32_t {
    // clang-format off
    Invalid        = 0,  ///< Will always be rejected
    None           = 1,  ///< No security applied
    Sign           = 2,  ///< All messages are signed but not encrypted
    SignAndEncrypt = 3,  ///< All messages are signed and encrypted
    // clang-format on
};

namespace detail {

using BuiltinTypes = std::tuple<
    UA_Boolean,
    UA_SByte,
    UA_Byte,
    UA_Int16,
    UA_UInt16,
    UA_Int32,
    UA_UInt32,
    UA_Int64,
    UA_UInt64,
    UA_Float,
    UA_Double,
    UA_String,
    UA_DateTime,
    UA_Guid,
    UA_ByteString,
    UA_XmlElement,
    UA_NodeId,
    UA_ExpandedNodeId,
    UA_StatusCode,
    UA_QualifiedName,
    UA_LocalizedText,
    UA_ExtensionObject,
    UA_DataValue,
    UA_Variant,
    UA_DiagnosticInfo>;

template <typename T>
constexpr bool isBuiltinType() {
    return TupleHolds<BuiltinTypes, T>::value;
}

// template <size_t Index>
// using BuiltinType = std::tuple_element<Index, BuiltinTypes>;

inline constexpr auto builtinTypesCount = std::tuple_size_v<BuiltinTypes>;

}  // namespace detail

}  // namespace opcua
