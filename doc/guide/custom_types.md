# Custom types {#custom_types}

@tableofcontents

Open62541pp provides two separate extension points for working with types that are not built into OPC UA:

- **TypeConverter** — map an existing C++ type to an already-defined OPC UA built-in type.
- **DataTypeBuilder** — define a brand-new composite OPC UA type (structure, union, or enumeration) that is encoded and communicated over the wire.

Choose the one that matches your situation. They can also be combined.

## TypeConverter {#custom_types-typeconverter}

Use opcua::TypeConverter when your C++ type has a natural mapping to one of the OPC UA built-in types (e.g. `UA_Byte`, `UA_Int32`, `UA_Float`).
Defining a specialization enables opcua::Variant to accept and return your type transparently via `assign()` and `to<T>()`.
No new OPC UA type is introduced on the wire; the existing built-in type is used for encoding.

**Specialization contract:**

```cpp
namespace opcua {

template <>
struct TypeConverter<MyType> {
    using NativeType = UA_???;                         // the built-in OPC UA type to map to
    static MyType  fromNative(const NativeType& src);  // native → C++
    static NativeType toNative(const MyType& src);     // C++ → native
};

}
```

Both functions should be `[[nodiscard]]`. Use value parameters for primitive types (`UA_Byte`, `UA_Int32`, …) and const-reference parameters for non-primitive types.

**Example** — mapping `std::byte` to `UA_Byte`:

```cpp
namespace opcua {

template <>
struct TypeConverter<std::byte> {
    using NativeType = UA_Byte;

    [[nodiscard]] static std::byte fromNative(UA_Byte src) noexcept {
        return std::byte{src};
    }

    [[nodiscard]] static UA_Byte toNative(std::byte src) noexcept {
        return std::to_integer<UA_Byte>(src);
    }
};

}
```

After this specialization, `std::byte` is a first-class citizen in opcua::Variant:

```cpp
opcua::Variant var;
var = std::byte{11};               // store
auto value = var.to<std::byte>();  // retrieve
```

## Custom DataTypes {#custom_types-datatypebuilder}

Use opcua::DataTypeBuilder when your data is a composite type (structure, union, or enumeration) that needs its own OPC UA type identity and binary encoding.
This is the correct approach when the server exposes a structured type that the client must deserialize, or when you publish your own structured variables.

### Struct layout requirements

The C++ struct passed to opcua::DataTypeBuilder must have a C-compatible memory layout:

- No virtual functions, no inheritance.
- No standard library containers (`std::vector`, `std::string`, …). Use the open62541 convention for dynamic arrays: a `size_t` count field immediately followed by a `T*` pointer field (see the `Measurements` example in `examples/custom_datatypes/`).
- Optional pointer fields (nullable) follow the same rule: a `nullptr` pointer means the field is absent.

### Registering the type description

Describe the binary layout of your C struct using opcua::DataTypeBuilder.
The resulting opcua::DataType object must be kept alive for the lifetime of the server or client that uses it.

**Structure:**

```cpp
struct Point { float x, y, z; };

const opcua::DataType& getPointDataType() {
    static const opcua::DataType dt =
        opcua::DataTypeBuilder<Point>::createStructure("Point", {1, 4242}, {1, 1})
            .addField<&Point::x>("x")
            .addField<&Point::y>("y")
            .addField<&Point::z>("z")
            .build();
    return dt;
}
```

**Enumeration** — the C++ enum must have an underlying type of `int32_t`:

```cpp
enum class Color : int32_t { Red = 0, Green = 1, Yellow = 2 };

const opcua::DataType& getColorDataType() {
    static const opcua::DataType dt =
        opcua::DataTypeBuilder<Color>::createEnum("Color", {1, 4946}, {1, 5}).build();
    return dt;
}
```

**Union:**

```cpp
enum class UniSwitch : uint32_t { None = 0, OptionA = 1, OptionB = 2 };

struct Uni {
    UniSwitch switchField;
    union { double optionA; UA_String optionB; } fields;
};

const opcua::DataType& getUniDataType() {
    static const opcua::DataType dt =
        opcua::DataTypeBuilder<Uni>::createUnion("Uni", {1, 4845}, {1, 4})
            .addUnionField<&Uni::fields, double>("optionA")
            .addUnionField<&Uni::fields, UA_String>("optionB", UA_TYPES[UA_TYPES_STRING])
            .build();
    return dt;
}
```

The two `NodeId` arguments to `create*` are the type's node id and its binary encoding node id in the OPC UA information model.
These must match the IDs used in the server's address space.

### Providing the description to server and client

Before starting the server or connecting the client, register all custom type descriptions via the config:

```cpp
opcua::Server server;
server.config().addCustomDataTypes({getPointDataType(), getColorDataType()});
```

```cpp
opcua::Client client;
client.config().addCustomDataTypes({getPointDataType(), getColorDataType()});
client.connect("opc.tcp://localhost:4840");
```

### Storing and reading values

Always pass the opcua::DataType when constructing a opcua::Variant from a custom type:

```cpp
const Point p{1.0f, 2.0f, 3.0f};
opcua::Variant var{p, getPointDataType()};
node.writeValue(var);
```

When reading, inspect the type before casting:

```cpp
opcua::Variant var = node.readValue();
if (var.isType(getPointDataType())) {
    const auto* p = static_cast<const Point*>(var.data());
}
```

@note Arrays of custom-type values are stored as arrays of opcua::ExtensionObject.
Open62541 unwraps scalars transparently, but arrays require manual iteration over the `ExtensionObject` elements.
See `examples/custom_datatypes/client_custom_datatypes.cpp` for a worked example.

## Combining both {#custom_types-combining}

TypeConverter and DataTypeBuilder address orthogonal concerns and can be used together.
For example, you can add a TypeConverter that maps `std::filesystem::path` to `UA_String`, and independently define a custom OPC UA structure that contains a `UA_String` field.

You can also add an opcua::TypeRegistry specialization so that template-deduced Variant construction works without repeating the DataType argument:

```cpp
namespace opcua {

template <>
struct TypeRegistry<Point> {
    static const UA_DataType& getDataType() noexcept {
        return *getPointDataType().handle();
    }
};

}
```

## Quick reference {#custom_types-reference}

| Question | Use |
|----------|-----|
| Map my C++ type to an existing UA built-in | TypeConverter |
| Define a new composite type encoded over the wire | DataTypeBuilder |
| Use `variant.to<T>()` with a custom DataType | TypeRegistry |
| Server exposes a structure I need to read | DataTypeBuilder + `addCustomDataTypes` |
