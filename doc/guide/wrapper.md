# Wrapper types {#wrapper}

@tableofcontents

Wrapper types in **open62541pp** provide a modern C++ interface to native open62541 objects. They maintain full compatibility with native types while adding features like RAII and pointer interconvertibility.

All wrapper classes derive from opcua::Wrapper. The template type opcua::TypeWrapper extends opcua::Wrapper class to safely copy, move and delete native types with heap-allocated memory.

## Access native object

The native object can be accessed in two ways:
- Using the opcua::Wrapper::handle() function
- Using the opcua::Wrapper::operator->() function to access members of the native type

```cpp
opcua::DataValue wrapper;

// Access native pointer
UA_DataValue* native = wrapper.handle();

// Access native members directly
wrapper->statusCode = UA_STATUSCODE_GOOD;  // Equivalent to:
wrapper.handle()->statusCode = UA_STATUSCODE_GOOD;
```

## Conversion

Both opcua::Wrapper and opcua::TypeWrapper are *pointer-interconvertible* with the wrapped native type.

Pointer interconvertibility enables lightweight casting between wrapper and native types without additional overhead.
This is achieved by ensuring the wrapper class is a standard-layout type, as required by the C++ standard.

> Two objects `a` and `b` are pointer-interconvertible if:
> One is a standard-layout class object [wrapper] and the other is the first non-static data
> member of that object [wrapped native type].
> Derived classes must fulfill the requirements of standard-layout types to be convertible.
> @see https://en.cppreference.com/w/cpp/language/static_cast#pointer-interconvertible

Following function allow seamless conversions between wrapper and native objects:

- Convert native objects to wrapper objects using:
  - opcua::asWrapper(NativeType\*)
  - opcua::asWrapper(const NativeType\*)
  - opcua::asWrapper(NativeType&)
  - opcua::asWrapper(const NativeType&)
- Convert wrapper objects to native objects using:
  - opcua::asNative(WrapperType\*)
  - opcua::asNative(const WrapperType\*)
  - opcua::asNative(WrapperType&)
  - opcua::asNative(const WrapperType&)


## Implicit conversion

Implicit conversions to the native object are provided for convenience:

```cpp
opcua::DataValue wrapper;
UA_DataValue& native = wrapper;

// Pass wrapper objects directly to functions expecting native types
void print(UA_DataValue& dv) { /* ... */ }
print(wrapper);                  // implicit conversion
print(opua::asNative(wrapper));  // explicit alternative
```

## Composed types

When wrapping composed types with nested members, getters and setters are essential for accessing these members as wrapper types.
Let's walk through an example to demonstrate how to implement such functionality.

### Example native type

```c
/**
 * ReadValueId
 * ^^^^^^^^^^^
 */
typedef struct {
    UA_NodeId nodeId;
    UA_UInt32 attributeId;
    UA_String indexRange;
    UA_QualifiedName dataEncoding;
} UA_ReadValueId;

#define UA_TYPES_READVALUEID 123
```

In this case, the struct contains heap-allocated members (`nodeId`, `indexRange`, and `dataEncoding`).
To manage these members correctly, our wrapper should inherit from opcua::TypeWrapper.

### Minimal wrapper class

A minimal wrapper implementation looks like this:

```cpp
class ReadValueId : public TypeWrapper<UA_ReadValueId, UA_TYPES_READVALUEID> {
public:
    // Inherit constructors
    using TypeWrapper::TypeWrapper;
};
```

We can now use the wrapper like this:

```
ReadValueId rv;
rv->attributeId = 1;
// How to set access nodeId?
```

### Adding getters and setters

To handle nested members like `nodeId`, we can use the opcua::asWrapper helper function to access these members as their corresponding wrapper types. Below is the enhanced wrapper class with getter and setter methods:

```cpp
class ReadValueId : public opcua::TypeWrapper<UA_ReadValueId, UA_TYPES_READVALUEID> {
public:
    // Inherit constructors
    using TypeWrapper::TypeWrapper;

    void setNodeId(opcua::NodeId nodeId) {
        asWrapper<opcua::NodeId>(handle()->nodeId) = std::move(nodeId);
    }

    opcua::NodeId& nodeId() const noexcept {
        return asWrapper<opcua::NodeId>(handle()->nodeId);
    }

    const opcua::NodeId& nodeId() noexcept {
        return asWrapper<opcua::NodeId>(handle()->nodeId);
    }

    uint32_t attributeId() const noexcept {
        return handle()->attributeId;
    }

    // ...
};
```

### Usage example

With these getters and setters, accessing and modifying `nodeId` becomes straightforward:

```cpp
ReadValueId rv;

// Set nodeId using the NodeId wrapper
rv.setNodeId(opcua::NodeId(1, "Identifier"));
// Access nodeId
opcua::NodeId& currentNodeId = rv.nodeId();
```
