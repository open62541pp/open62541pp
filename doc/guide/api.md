# Choosing the right API {#api}

@tableofcontents

Open62541pp provides two complementary APIs for interacting with the OPC UA address space.
Understanding when to use each one avoids confusion and leads to cleaner code.

## The Node API {#api-node}

opcua::Node is a handle to a single node identified by a opcua::NodeId.
It offers a concise, object-oriented interface and throws opcua::BadStatus exceptions on failure.

```cpp
opcua::Node node{client, opcua::NodeId{1, 1000}};
node.writeValue(opcua::Variant{42});                  // throws on error
auto value = node.readValue().to<int>();              // throws on error
auto child = node.addVariable({1, 2000}, "Counter");  // returns a Node
```

**Use the Node API when:**
- writing sequential, synchronous code where exceptions are acceptable
- operating on individual nodes one at a time
- building scripts, tools, or quick integrations

## The services namespace {#api-services}

The free functions in `opcua::services` mirror the OPC UA service set directly.
They accept any combination of opcua::Server or opcua::Client and always return opcua::Result<T> — never throw.
Asynchronous variants (suffixed `Async`) are also available here.

```cpp
// Result<T> — inspect the status code without catching exceptions
opcua::Result<opcua::Variant> result = opcua::services::readValue(client, id);
if (result) {
    int v = result->to<int>();
}

// Batched read — one round-trip for multiple attributes
opcua::ReadResponse response = opcua::services::read(
    client,
    {{id1, opcua::AttributeId::Value}, {id2, opcua::AttributeId::Value}},
    opcua::TimestampsToReturn::Both
);

// Async — completion token, future, or deferred
opcua::services::readValueAsync(client, id, [](opcua::Result<opcua::Variant>& result) {
    // called from the event loop
});
```

**Use the services namespace when:**
- you need `Result<T>` to compose or chain error handling without exceptions
- performing batched requests (multiple nodes in one round-trip)
- using callbacks, deferred execution, or custom completion tokens (full async model)
- accessing response fields not exposed by the Node API

## Mixing both APIs {#api-mixing}

The two APIs are fully interchangeable.
opcua::Node is a thin wrapper around the same service functions; there is no performance difference.
You can obtain the opcua::NodeId from any node and pass it directly to service functions:

```cpp
opcua::Node node{server, opcua::ObjectId::ObjectsFolder};
// switch to services for a batch operation
opcua::VariableAttributes attrs;
opcua::services::addVariable(
    server, node.id(), {1, 1000}, "Speed", attrs,
    opcua::VariableTypeId::BaseDataVariableType,
    opcua::ReferenceTypeId::HasComponent
);
```

## Quick reference {#api-reference}

| Criterion | Node API | services namespace |
|-----------|----------|--------------------|
| Error model | throws `BadStatus` | returns `Result<T>` |
| Async support | partial — `std::future` only | full — callbacks, futures, deferred, custom tokens |
| Batching | no | yes |
| Works on Server | yes | yes |
| Works on Client | yes | yes |
| Verbosity | lower | higher |
