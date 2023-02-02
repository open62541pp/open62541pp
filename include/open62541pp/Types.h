#pragma once

#include <cstdint>
#include <string_view>
#include <tuple>

#include "open62541pp/Traits.h"
#include "open62541pp/open62541.h"

namespace opcua {

/**
 * Built-in types.
 * @see https://reference.opcfoundation.org/v104/Core/docs/Part6/5.1.2/
 */
enum class Type : uint16_t {
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
 * Node classes.
 * @see UA_NodeClass
 */
enum class NodeClass : uint16_t {
    // clang-format off
    Unspecified   = UA_NODECLASS_UNSPECIFIED,
    Object        = UA_NODECLASS_OBJECT,
    Variable      = UA_NODECLASS_VARIABLE,
    Method        = UA_NODECLASS_METHOD,
    ObjectType    = UA_NODECLASS_OBJECTTYPE,
    VariableType  = UA_NODECLASS_VARIABLETYPE,
    ReferenceType = UA_NODECLASS_REFERENCETYPE,
    DataType      = UA_NODECLASS_DATATYPE,
    View          = UA_NODECLASS_VIEW,
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
 * Reference types.
 *
 * List in standard: https://reference.opcfoundation.org/v104/Core/ReferenceTypes/
 * Missing reference types in open62541?
 * - AliasFor
 * - HasReaderGroup
 * - HasWriterGroup
 */
enum class ReferenceType : uint16_t {
    // clang-format off
    References                          = UA_NS0ID_REFERENCES,
    NonHierarchicalReferences           = UA_NS0ID_NONHIERARCHICALREFERENCES,
    HierarchicalReferences              = UA_NS0ID_HIERARCHICALREFERENCES,
    HasChild                            = UA_NS0ID_HASCHILD,
    Organizes                           = UA_NS0ID_ORGANIZES,
    HasEventSource                      = UA_NS0ID_HASEVENTSOURCE,
    HasModellingRule                    = UA_NS0ID_HASMODELLINGRULE,
    HasEncoding                         = UA_NS0ID_HASENCODING,
    HasDescription                      = UA_NS0ID_HASDESCRIPTION,
    HasTypeDefinition                   = UA_NS0ID_HASTYPEDEFINITION,
    GeneratesEvent                      = UA_NS0ID_GENERATESEVENT,
    Aggregates                          = UA_NS0ID_AGGREGATES,
    HasSubType                          = UA_NS0ID_HASSUBTYPE,
    HasProperty                         = UA_NS0ID_HASPROPERTY,
    HasComponent                        = UA_NS0ID_HASCOMPONENT,
    HasNotifier                         = UA_NS0ID_HASNOTIFIER,
    HasOrderedComponent                 = UA_NS0ID_HASORDEREDCOMPONENT,
    FromState                           = UA_NS0ID_FROMSTATE,
    ToState                             = UA_NS0ID_TOSTATE,
    HasCause                            = UA_NS0ID_HASCAUSE,
    HasEffect                           = UA_NS0ID_HASEFFECT,
    HasHistoricalConfiguration          = UA_NS0ID_HASHISTORICALCONFIGURATION,
    HasSubStateMachine                  = UA_NS0ID_HASSUBSTATEMACHINE,
    HasArgumentDescription              = UA_NS0ID_HASARGUMENTDESCRIPTION,
    HasOptionalInputArgumentDescription = UA_NS0ID_HASOPTIONALINPUTARGUMENTDESCRIPTION,
    AlwaysGeneratesEvent                = UA_NS0ID_ALWAYSGENERATESEVENT,
    HasTrueSubState                     = UA_NS0ID_HASTRUESUBSTATE,
    HasFalseSubState                    = UA_NS0ID_HASFALSESUBSTATE,
    HasCondition                        = UA_NS0ID_HASCONDITION,
    HasPubSubConnection                 = UA_NS0ID_HASPUBSUBCONNECTION,
    DataSetToWriter                     = UA_NS0ID_DATASETTOWRITER,
    HasGuard                            = UA_NS0ID_HASGUARD,
    HasDataSetWriter                    = UA_NS0ID_HASDATASETWRITER,
    HasDataSetReader                    = UA_NS0ID_HASDATASETREADER,
    HasAlarmSuppressionGroup            = UA_NS0ID_HASALARMSUPPRESSIONGROUP,
    AlarmGroupMember                    = UA_NS0ID_ALARMGROUPMEMBER,
    HasEffectDisable                    = UA_NS0ID_HASEFFECTDISABLE,
    HasDictionaryEntry                  = UA_NS0ID_HASDICTIONARYENTRY,
    HasInterface                        = UA_NS0ID_HASINTERFACE,
    HasAddIn                            = UA_NS0ID_HASADDIN,
    HasEffectEnable                     = UA_NS0ID_HASEFFECTENABLE,
    HasEffectSuppressed                 = UA_NS0ID_HASEFFECTSUPPRESSED,
    HasEffectUnsuppressed               = UA_NS0ID_HASEFFECTUNSUPPRESSED,
    // clang-format on
};

namespace detail {

using NativeTypes = std::tuple<
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
constexpr bool isNativeType() {
    return TupleHolds<NativeTypes, T>::value;
}

// template <size_t Index>
// using NativeType = std::tuple_element<Index, NativeTypes>;

}  // namespace detail

}  // namespace opcua
