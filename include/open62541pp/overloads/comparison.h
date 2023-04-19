#pragma once

#include "open62541pp/open62541.h"

/* ------------------------------------------ UA_String ----------------------------------------- */

inline bool operator==(const UA_String& left, const UA_String& right) noexcept {
    return UA_String_equal(&left, &right);
}

inline bool operator!=(const UA_String& left, const UA_String& right) noexcept {
    return !(left == right);
}

/* ------------------------------------------- UA_Guid ------------------------------------------ */

inline bool operator==(const UA_Guid& left, const UA_Guid& right) noexcept {
    return UA_Guid_equal(&left, &right);
}

inline bool operator!=(const UA_Guid& left, const UA_Guid& right) noexcept {
    return !(left == right);
}

/* ------------------------------------------ UA_NodeId ----------------------------------------- */

inline bool operator==(const UA_NodeId& left, const UA_NodeId& right) noexcept {
    return UA_NodeId_equal(&left, &right);
}

inline bool operator!=(const UA_NodeId& left, const UA_NodeId& right) noexcept {
    return !(left == right);
}

inline bool operator<(const UA_NodeId& left, const UA_NodeId& right) noexcept {
    return UA_NodeId_order(&left, &right) == UA_ORDER_LESS;
}

inline bool operator>(const UA_NodeId& left, const UA_NodeId& right) noexcept {
    return UA_NodeId_order(&left, &right) == UA_ORDER_MORE;
}

inline bool operator<=(const UA_NodeId& left, const UA_NodeId& right) noexcept {
    return (left < right) || (left == right);
}

inline bool operator>=(const UA_NodeId& left, const UA_NodeId& right) noexcept {
    return (left > right) || (left == right);
}

/* -------------------------------------- UA_ExpandedNodeId ------------------------------------- */

inline bool operator==(const UA_ExpandedNodeId& left, const UA_ExpandedNodeId& right) noexcept {
    return UA_ExpandedNodeId_equal(&left, &right);
}

inline bool operator!=(const UA_ExpandedNodeId& left, const UA_ExpandedNodeId& right) noexcept {
    return !(left == right);
}

inline bool operator<(const UA_ExpandedNodeId& left, const UA_ExpandedNodeId& right) noexcept {
    return UA_ExpandedNodeId_order(&left, &right) == UA_ORDER_LESS;
}

inline bool operator>(const UA_ExpandedNodeId& left, const UA_ExpandedNodeId& right) noexcept {
    return UA_ExpandedNodeId_order(&left, &right) == UA_ORDER_MORE;
}

inline bool operator<=(const UA_ExpandedNodeId& left, const UA_ExpandedNodeId& right) noexcept {
    return (left < right) || (left == right);
}

inline bool operator>=(const UA_ExpandedNodeId& left, const UA_ExpandedNodeId& right) noexcept {
    return (left > right) || (left == right);
}

/* -------------------------------------- UA_QualifiedName -------------------------------------- */

inline bool operator==(const UA_QualifiedName& left, const UA_QualifiedName& right) noexcept {
    return (left.namespaceIndex == right.namespaceIndex) && (left.name == right.name);
}

inline bool operator!=(const UA_QualifiedName& left, const UA_QualifiedName& right) noexcept {
    return !(left == right);
}

/* -------------------------------------- UA_LocalizedText -------------------------------------- */

inline bool operator==(const UA_LocalizedText& left, const UA_LocalizedText& right) noexcept {
    return (left.locale == right.locale) && (left.text == right.text);
}

inline bool operator!=(const UA_LocalizedText& left, const UA_LocalizedText& right) noexcept {
    return !(left == right);
}
