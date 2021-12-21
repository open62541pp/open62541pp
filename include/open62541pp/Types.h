#pragma once

#include <type_traits>

#include "open62541pp/open62541.h"

namespace opcua {

enum class Type : uint16_t {
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
};

enum class NodeClass : uint16_t {
    Unspecified   = UA_NODECLASS_UNSPECIFIED,
    Object        = UA_NODECLASS_OBJECT,
    Variable      = UA_NODECLASS_VARIABLE,
    Method        = UA_NODECLASS_METHOD,
    ObjectType    = UA_NODECLASS_OBJECTTYPE,
    VariableType  = UA_NODECLASS_VARIABLETYPE,
    ReferenceType = UA_NODECLASS_REFERENCETYPE,
    DataType      = UA_NODECLASS_DATATYPE,
    View          = UA_NODECLASS_VIEW,
};

// #define UA_NS0ID_REFERENCES 31 // ReferenceType
// #define UA_NS0ID_NONHIERARCHICALREFERENCES 32 // ReferenceType
// #define UA_NS0ID_HIERARCHICALREFERENCES 33 // ReferenceType
// #define UA_NS0ID_HASCHILD 34 // ReferenceType
// #define UA_NS0ID_ORGANIZES 35 // ReferenceType
// #define UA_NS0ID_HASEVENTSOURCE 36 // ReferenceType
// #define UA_NS0ID_HASMODELLINGRULE 37 // ReferenceType
// #define UA_NS0ID_HASENCODING 38 // ReferenceType
// #define UA_NS0ID_HASDESCRIPTION 39 // ReferenceType
// #define UA_NS0ID_HASTYPEDEFINITION 40 // ReferenceType
// #define UA_NS0ID_GENERATESEVENT 41 // ReferenceType
// #define UA_NS0ID_AGGREGATES 44 // ReferenceType
// #define UA_NS0ID_HASSUBTYPE 45 // ReferenceType
// #define UA_NS0ID_HASPROPERTY 46 // ReferenceType
// #define UA_NS0ID_HASCOMPONENT 47 // ReferenceType
// #define UA_NS0ID_HASNOTIFIER 48 // ReferenceType
// #define UA_NS0ID_HASORDEREDCOMPONENT 49 // ReferenceType
// #define UA_NS0ID_FROMSTATE 51 // ReferenceType
// #define UA_NS0ID_TOSTATE 52 // ReferenceType
// #define UA_NS0ID_HASCAUSE 53 // ReferenceType
// #define UA_NS0ID_HASEFFECT 54 // ReferenceType
// #define UA_NS0ID_HASHISTORICALCONFIGURATION 56 // ReferenceType

enum class ReferenceType : uint16_t {
    Organizes    = UA_NS0ID_ORGANIZES,
    HasSubtype   = UA_NS0ID_HASSUBTYPE,
    HasComponent = UA_NS0ID_HASCOMPONENT
};

// helper trait
template <typename...> struct always_false : std::false_type {};

template <typename T> inline constexpr Type getType() {
    static_assert(always_false<T>::value,
                  "Type mapping not possible (maybe not existing or not unique). \
                   Please specify type manually.");
    return {}; // TODO: Type::Undefined?
}

template <> inline constexpr Type getType<UA_Boolean>()         { return Type::Boolean; }
template <> inline constexpr Type getType<UA_SByte>()           { return Type::SByte; }
template <> inline constexpr Type getType<UA_Byte>()            { return Type::Byte; }
template <> inline constexpr Type getType<UA_Int16>()           { return Type::Int16; }
template <> inline constexpr Type getType<UA_UInt16>()          { return Type::UInt16; }
template <> inline constexpr Type getType<UA_Int32>()           { return Type::Int32; }
template <> inline constexpr Type getType<UA_UInt32>()          { return Type::UInt32; }
template <> inline constexpr Type getType<UA_Int64>()           { return Type::Int64; }
template <> inline constexpr Type getType<UA_UInt64>()          { return Type::UInt64; }
template <> inline constexpr Type getType<UA_Float>()           { return Type::Float; }
template <> inline constexpr Type getType<UA_Double>()          { return Type::Double; }
template <> inline constexpr Type getType<UA_String>()          { return Type::String; }
// template <> inline constexpr Type getType<UA_DateTime>()        { return Type::DateTime; }
template <> inline constexpr Type getType<UA_Guid>()            { return Type::Guid; }
// template <> inline constexpr Type getType<UA_ByteString>()      { return Type::ByteString; }
// template <> inline constexpr Type getType<UA_XmlElement>()      { return Type::XmlElement; }
template <> inline constexpr Type getType<UA_NodeId>()          { return Type::NodeId; }
// template <> inline constexpr Type getType<UA_ExpanededNodeId>() { return Type::ExpanededNodeId; }
// template <> inline constexpr Type getType<UA_StatusCode>()      { return Type::StatusCode; }
template <> inline constexpr Type getType<UA_QualifiedName>()   { return Type::QualifiedName; }
template <> inline constexpr Type getType<UA_LocalizedText>()   { return Type::LocalizedText; }
template <> inline constexpr Type getType<UA_ExtensionObject>() { return Type::ExtensionObject; }
template <> inline constexpr Type getType<UA_DataValue>()       { return Type::DataValue; }
template <> inline constexpr Type getType<UA_Variant>()         { return Type::Variant; }
template <> inline constexpr Type getType<UA_DiagnosticInfo>()  { return Type::DiagnosticInfo; }

// Get UA_DataType by template argument
template <typename T>
inline const UA_DataType* getUaDataType() {
    return &UA_TYPES[static_cast<uint16_t>(getType<T>())];
}

// Get UA_DataType by Type enum
inline const UA_DataType* getUaDataType(Type type) {
    return &UA_TYPES[static_cast<uint16_t>(type)];
}

} // namespace opcua
