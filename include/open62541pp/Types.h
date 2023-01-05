#pragma once

#include <type_traits>

#include "open62541pp/open62541.h"

namespace opcua {

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
    ExpanededNodeId = UA_TYPES_EXPANDEDNODEID,
    StatusCode      = UA_TYPES_STATUSCODE,
    QualifiedName   = UA_TYPES_QUALIFIEDNAME,
    LocalizedText   = UA_TYPES_LOCALIZEDTEXT,
    ExtensionObject = UA_TYPES_EXTENSIONOBJECT,
    DataValue       = UA_TYPES_DATAVALUE,
    Variant         = UA_TYPES_VARIANT,
    DiagnosticInfo  = UA_TYPES_DIAGNOSTICINFO
    // clang-format on
};

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
    HasSubtype                          = UA_NS0ID_HASSUBTYPE,
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

template <typename...>
struct always_false : std::false_type {};

}  // namespace detail

template <typename T>
constexpr Type getType() {
    static_assert(
        detail::always_false<T>::value,
        "Type mapping not possible (maybe not existing or not unique). "
        "Please specify type manually."
    );
    return {};  // TODO: Type::Undefined?
}

// clang-format off
template <> constexpr Type getType<UA_Boolean>()         { return Type::Boolean; }
template <> constexpr Type getType<UA_SByte>()           { return Type::SByte; }
template <> constexpr Type getType<UA_Byte>()            { return Type::Byte; }
template <> constexpr Type getType<UA_Int16>()           { return Type::Int16; }
template <> constexpr Type getType<UA_UInt16>()          { return Type::UInt16; }
template <> constexpr Type getType<UA_Int32>()           { return Type::Int32; }
template <> constexpr Type getType<UA_UInt32>()          { return Type::UInt32; }
template <> constexpr Type getType<UA_Int64>()           { return Type::Int64; }
template <> constexpr Type getType<UA_UInt64>()          { return Type::UInt64; }
template <> constexpr Type getType<UA_Float>()           { return Type::Float; }
template <> constexpr Type getType<UA_Double>()          { return Type::Double; }
template <> constexpr Type getType<UA_String>()          { return Type::String; }
// template <> constexpr Type getType<UA_DateTime>()        { return Type::DateTime; }
template <> constexpr Type getType<UA_Guid>()            { return Type::Guid; }
// template <> constexpr Type getType<UA_ByteString>()      { return Type::ByteString; }
// template <> constexpr Type getType<UA_XmlElement>()      { return Type::XmlElement; }
template <> constexpr Type getType<UA_NodeId>()          { return Type::NodeId; }
// template <> constexpr Type getType<UA_ExpanededNodeId>() { return Type::ExpanededNodeId; }
// template <> constexpr Type getType<UA_StatusCode>()      { return Type::StatusCode; }
template <> constexpr Type getType<UA_QualifiedName>()   { return Type::QualifiedName; }
template <> constexpr Type getType<UA_LocalizedText>()   { return Type::LocalizedText; }
template <> constexpr Type getType<UA_ExtensionObject>() { return Type::ExtensionObject; }
template <> constexpr Type getType<UA_DataValue>()       { return Type::DataValue; }
template <> constexpr Type getType<UA_Variant>()         { return Type::Variant; }
template <> constexpr Type getType<UA_DiagnosticInfo>()  { return Type::DiagnosticInfo; }

// clang-format on

/// Get UA_DataType by template argument.
template <typename T>
inline const UA_DataType* getUaDataType() {
    return &UA_TYPES[static_cast<uint16_t>(getType<T>())];  // NOLINT
}

/// Get UA_DataType by Type enum.
inline const UA_DataType* getUaDataType(Type type) {
    return &UA_TYPES[static_cast<uint16_t>(type)];  // NOLINT
}

}  // namespace opcua
