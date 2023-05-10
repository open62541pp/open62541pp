#pragma once

// public open62541 headers needed by open62541++

#ifndef _MSC_VER
// ignore compile warnings of open62541 v1.3:
// - missing initializer for member ‘UA_NodeId::identifier’
// - missing initializer for member ‘UA_ExpandedNodeId::namespaceUri’
// - missing initializer for member ‘UA_ExpandedNodeId::serverIndex’
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#if __has_include(<open62541.h>)
// UA_ENABLE_AMALGAMATION=ON
#include <open62541.h>
#else
// UA_ENABLE_AMALGAMATION=OFF
#include <open62541/nodeids.h>
#include <open62541/types.h>
#include <open62541/types_generated.h>
#endif

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
