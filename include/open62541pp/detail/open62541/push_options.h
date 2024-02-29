// no include guard

#if defined(__GNUC__)
// ignore compile warnings of open62541:
// - missing initializer for member ‘UA_NodeId::identifier’
// - missing initializer for member ‘UA_ExpandedNodeId::namespaceUri’
// - missing initializer for member ‘UA_ExpandedNodeId::serverIndex’
// - suggest braces around initialization of subobject [-Wmissing-braces]
//   UA_ExpandedNodeId id = {0}; id.nodeId = nodeId; return id;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wmissing-braces"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
