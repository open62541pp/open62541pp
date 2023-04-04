#pragma once

#ifndef _MSC_VER
// ignore compile warnings of open62541 v1.3:
// - missing initializer for member ‘UA_NodeId::identifier’
// - missing initializer for member ‘UA_ExpandedNodeId::namespaceUri’
// - missing initializer for member ‘UA_ExpandedNodeId::serverIndex’
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

// public open62541 headers needed by open62541++
#include <open62541/nodeids.h>
#include <open62541/types.h>
#include <open62541/types_generated.h>

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
