# open62541++

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-blue.svg)](https://github.com/open62541pp/open62541pp/blob/master/LICENSE)
[![CI](https://github.com/open62541pp/open62541pp/actions/workflows/ci.yml/badge.svg)](https://github.com/open62541pp/open62541pp/actions/workflows/ci.yml)
[![Compatibility](https://github.com/open62541pp/open62541pp/actions/workflows/open62541-compatibility.yml/badge.svg)](https://github.com/open62541pp/open62541pp/actions/workflows/open62541-compatibility.yml)
[![Coverage](https://codecov.io/github/open62541pp/open62541pp/branch/master/graph/badge.svg?token=P87N1WRXC4)](https://codecov.io/github/open62541pp/open62541pp)
[![Documentation](https://github.com/open62541pp/open62541pp/actions/workflows/doc.yml/badge.svg)](https://github.com/open62541pp/open62541pp/actions/workflows/doc.yml)

**[Documentation](https://open62541pp.github.io/open62541pp) · [Examples](https://github.com/open62541pp/open62541pp/tree/master/examples)**

open62541++ is a C++ wrapper built on top of the amazing [open62541](https://open62541.org) OPC UA (OPC Unified Architecture) library.

Features and goals:

- High-level and easy to use classes similar to the [python-opcua API](https://python-opcua.readthedocs.io):
  - `opcua::Server`
  - `opcua::Client`
  - `opcua::Node`
- Safe wrapper classes for open62541 `UA_*` types to prevent memory leaks
- Native open62541 objects can be accessed using the `handle()` method of the wrapping classes
- Extensible type conversion system to convert arbitrary types to/from native `UA_*` types
- Cross-platform (tested on Windows, Linux and MacOS)
- Compatible with all stable open62541 versions (> v1.0)
- Easy installation and integration with CMake
- Use modern C++ (C++ 17) and best practices
- Less hurdle to get started with OPC UA

## Examples

### Server

<!-- [[[cog
from pathlib import Path
import cog
cog.outl("```cpp")
cog.out(Path("examples/server_minimal.cpp").read_text())
cog.outl("```")
]]] -->
```cpp
#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server;

    // add variable node
    auto parentNode = server.getObjectsNode();
    auto myIntegerNode = parentNode.addVariable({1, "the.answer"}, "the answer");
    // set node attributes
    myIntegerNode.writeDataType(opcua::Type::Int32);
    myIntegerNode.writeDisplayName({"en-US", "the answer"});
    myIntegerNode.writeDescription({"en-US", "the answer"});
    myIntegerNode.writeScalar(42);

    server.run();
}
```
<!-- [[[end]]] -->

### Client

<!-- [[[cog
from pathlib import Path
import cog
cog.outl("```cpp")
cog.out(Path("examples/client_minimal.cpp").read_text())
cog.outl("```")
]]] -->
```cpp
#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Client client;
    client.connect("opc.tcp://localhost:4840");

    auto node = client.getNode({0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME});
    const auto dt = node.readScalar<opcua::DateTime>();

    std::cout << "Server date (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;
}
```
<!-- [[[end]]] -->

## Type conversion

Type conversion from and to native `UA_*` types are handled by the `opcua::TypeConverter` struct.

Compile-time checks are used where possible:

```cpp
opcua::Variant var;

// will compile
int number = 5;
var.setScalar(number);
var.setArrayCopy<double>({1.1, 2.2, 3.3});

// won't compile, because the std::string can't be assigned without copy (conversion needed)
std::string str{"test"};
var.setScalar(str);

// won't compile, because the type std::string is associated with more than one variant types:
// - opcua::Type::String
// - opcua::Type::ByteString
// - opcua::Type::XmlElement
var.setScalarCopy<std::string>("test");

// finally compiles
var.setScalarCopy<std::string, opcua::Type::String>("test");
```

You can add template specializations to add conversions for arbitrary types:

```cpp
namespace opcua {

template <>
struct TypeConverter<std::string> {
    using ValueType = std::string;
    using NativeType = UA_String;
    using ValidTypes = TypeList<Type::String, Type::ByteString, Type::XmlElement>;

    static void fromNative(const NativeType& src, ValueType& dst) { /* ... */ }
    static void toNative(const ValueType& src, NativeType& dst) { /* ... */ }
};

}  // namespace opcua
```

### Type map of built-in types

| Type Enum `opcua::Type`  | Type                 | Typedef     | Wrapper                           | Conversions               |
| ------------------------ | -------------------- | ----------- | --------------------------------- | ------------------------- |
| Boolean                  | `UA_Boolean`         | `bool`      |                                   |                           |
| SByte                    | `UA_SByte`           | `int8_t`    |                                   |                           |
| Byte                     | `UA_Byte`            | `uint8_t`   |                                   |                           |
| Int16                    | `UA_Int16`           | `int16_t`   |                                   |                           |
| UInt16                   | `UA_UInt16`          | `uint16_t`  |                                   |                           |
| Int32                    | `UA_Int32`           | `int32_t`   |                                   |                           |
| UInt32                   | `UA_UInt32`          | `uint32_t`  |                                   |                           |
| Int64                    | `UA_Int64`           | `int64_t`   |                                   |                           |
| UInt64                   | `UA_UInt64`          | `uint64_t`  |                                   |                           |
| Float                    | `UA_Float`           | `float`     |                                   |                           |
| Double                   | `UA_Double`          | `double`    |                                   |                           |
| String                   | `UA_String`          |             | `opcua::String`                   | `std::string`             |
| DateTime                 | `UA_DateTime`        | `int64_t`   | `opcua::DateTime`                 | `std::chrono::time_point` |
| Guid                     | `UA_Guid`            |             | `opcua::Guid`                     |                           |
| ByteString               | `UA_ByteString`      | `UA_String` | `opcua::ByteString`               | `std::string`             |
| XmlElement               | `UA_XmlElement`      | `UA_String` | `opcua::XmlElement`               | `std::string`             |
| NodeId                   | `UA_NodeId`          |             | `opcua::NodeId`                   |                           |
| ExpandedNodeId           | `UA_ExpandedNodeId`  |             | `opcua::ExpandedNodeId`           |                           |
| StatusCode               | `UA_StatusCode`      | `uint32_t`  |                                   |                           |
| QualifiedName            | `UA_QualifiedName`   |             | `opcua::QualifiedName`            |                           |
| LocalizedText            | `UA_LocalizedText`   |             | `opcua::LocalizedText`            |                           |
| ExtensionObjectDataValue | `UA_ExtensionObject` |             | `opcua::ExtensionObjectDataValue` |                           |
| DataValue                | `UA_DataValue`       |             | `opcua::DataValue`                |                           |
| Variant                  | `UA_Variant`         |             | `opcua::Variant`                  |                           |
| DiagnosticInfo           | `UA_DiagnosticInfo`  |             | `opcua::DiagnosticInfo`           |                           |

## Install

The library can be installed using [CMake](https://cmake.org/runningcmake/).
Add it to your project as a Git submodule (`git submodule add https://github.com/open62541pp/open62541pp.git`) and link it with CMake:

```cmake
add_subdirectory(extern/open62541pp)  # the submodule directory
target_link_library(myexecutable PRIVATE open62541pp::open62541pp)
```

Please check out the open62541 build options here: https://www.open62541.org/doc/1.3/building.html#build-options

open62541++ provides additional build options:

- `UAPP_INTERNAL_OPEN62541`: Use internal open62541 library if `ON` or search for installed open62541 library if `OFF`
- `UAPP_BUILD_DOCUMENTATION`: Build documentation
- `UAPP_BUILD_EXAMPLES`: Build examples for `examples` directory
- `UAPP_BUILD_TESTS`: Build an run tests
- `UAPP_ENABLE_CLANG_TIDY`: Enable static code analysis with Clang-Tidy
- `UAPP_ENABLE_COVERAGE`: Enable coverage analysis
- `UAPP_ENABLE_SANITIZER_ADDRESS/LEAK/MEMORY/THREAD/UNDEFINED_BEHAVIOUR`: Enable sanitizers

### Dependencies

- [open62541](https://github.com/open62541/open62541) as integrated submodule or external dependency
- [catch2](https://github.com/catchorg/Catch2) for tests
