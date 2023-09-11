<!-- links to documentation -->
[doc-server]: https://open62541pp.github.io/open62541pp/classopcua_1_1Server.html
[doc-client]: https://open62541pp.github.io/open62541pp/classopcua_1_1Client.html
[doc-node]: https://open62541pp.github.io/open62541pp/classopcua_1_1Node.html
[doc-typewrapper]: https://open62541pp.github.io/open62541pp/group__TypeWrapper.html

<div align="center">
  <h1>open62541++</h1>

  [![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-blue.svg)](https://github.com/open62541pp/open62541pp/blob/master/LICENSE)
  [![CI](https://github.com/open62541pp/open62541pp/actions/workflows/ci.yml/badge.svg)](https://github.com/open62541pp/open62541pp/actions/workflows/ci.yml)
  [![Compatibility](https://github.com/open62541pp/open62541pp/actions/workflows/open62541-compatibility.yml/badge.svg)](https://github.com/open62541pp/open62541pp/actions/workflows/open62541-compatibility.yml)
  [![Package](https://github.com/open62541pp/open62541pp/actions/workflows/package.yml/badge.svg)](https://github.com/open62541pp/open62541pp/actions/workflows/package.yml)
  [![Documentation](https://github.com/open62541pp/open62541pp/actions/workflows/doc.yml/badge.svg)](https://github.com/open62541pp/open62541pp/actions/workflows/doc.yml)
  [![Coverage](https://codecov.io/gh/open62541pp/open62541pp/branch/master/graph/badge.svg?token=P87N1WRXC4)](https://codecov.io/gh/open62541pp/open62541pp)

  <h3>C++ wrapper of the amazing <a href="https://open62541.org">open62541</a> OPC UA library.</h3>

  <p>
    <b>
      <a href="https://open62541pp.github.io/open62541pp">Documentation</a>
      •
      <a href="https://github.com/open62541pp/open62541pp/tree/master/examples">Examples</a>
    </b>
  </p>
</div>

## 🎯 Features and goals

- High-level and easy to use classes similar to the [Python opcua/asyncua API](https://python-opcua.readthedocs.io):
  - [`opcua::Server`][doc-server]
  - [`opcua::Client`][doc-client]
  - [`opcua::Node`][doc-node]
- [Safe wrapper classes][doc-typewrapper] for open62541 `UA_*` types to prevent memory leaks
- Native open62541 objects can be accessed using the `handle()` method of the wrapping classes
- [Extensible type conversion system](#-type-conversion) to convert arbitrary types to/from native `UA_*` types
- Cross-platform (tested on Windows, Linux and macOS)
- Compatible with all stable open62541 versions ≥ v1.0
- [Easy installation and integration with CMake](#-getting-started)
- Use modern C++ (C++ 17) and best practices
- Less hurdle to get started with OPC UA

## ✍ Examples

[Explore all examples in the `examples/` directory](https://github.com/open62541pp/open62541pp/tree/master/examples).

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

    // Add a variable node to the Objects node
    opcua::Node parentNode = server.getObjectsNode();
    opcua::Node myIntegerNode = parentNode.addVariable({1, 1000}, "TheAnswer");
    // Write some node attributes
    myIntegerNode.writeDisplayName({"en-US", "The Answer"})
        .writeDataType(opcua::DataTypeId::Int32)
        .writeValueScalar(42);

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

    opcua::Node node = client.getNode(opcua::VariableId::Server_ServerStatus_CurrentTime);
    const auto dt = node.readValueScalar<opcua::DateTime>();

    std::cout << "Server date (UTC): " << dt.format("%Y-%m-%d %H:%M:%S") << std::endl;
}
```
<!-- [[[end]]] -->

## ⇆ Type conversion

Type conversion from and to native `UA_*` types are handled by the `opcua::TypeConverter` struct.
Have a look at the [typeconversion example](https://github.com/open62541pp/open62541pp/blob/master/examples/typeconversion.cpp).

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

// will compile
var.setScalarCopy(str);
```

You can add template specializations to add conversions for arbitrary types:

```cpp
namespace opcua {

template <>
struct TypeConverter<std::string> {
    using ValueType = std::string;
    using NativeType = UA_String;
    using ValidTypes = TypeIndexList<UA_TYPES_STRING, UA_TYPES_BYTESTRING, UA_TYPES_XMLELEMENT>;

    static void fromNative(const NativeType& src, ValueType& dst) { /* ... */ }
    static void toNative(const ValueType& src, NativeType& dst) { /* ... */ }
};

}  // namespace opcua
```

### Type map of built-in types

| Type Enum `opcua::Type`  | Type                 | Typedef     | Wrapper                           | Conversions                                                 |
| ------------------------ | -------------------- | ----------- | --------------------------------- | ----------------------------------------------------------- |
| Boolean         | `UA_Boolean`         | `bool`      |                          |                                                             |
| SByte           | `UA_SByte`           | `int8_t`    |                          |                                                             |
| Byte            | `UA_Byte`            | `uint8_t`   |                          |                                                             |
| Int16           | `UA_Int16`           | `int16_t`   |                          |                                                             |
| UInt16          | `UA_UInt16`          | `uint16_t`  |                          |                                                             |
| Int32           | `UA_Int32`           | `int32_t`   |                          |                                                             |
| UInt32          | `UA_UInt32`          | `uint32_t`  |                          |                                                             |
| Int64           | `UA_Int64`           | `int64_t`   |                          |                                                             |
| UInt64          | `UA_UInt64`          | `uint64_t`  |                          |                                                             |
| Float           | `UA_Float`           | `float`     |                          |                                                             |
| Double          | `UA_Double`          | `double`    |                          |                                                             |
| String          | `UA_String`          |             | `opcua::String`          | `std::string`, `std::string_view`, `const char*`, `char[N]` |
| DateTime        | `UA_DateTime`        | `int64_t`   | `opcua::DateTime`        | `std::chrono::time_point`                                   |
| Guid            | `UA_Guid`            |             | `opcua::Guid`            |                                                             |
| ByteString      | `UA_ByteString`      | `UA_String` | `opcua::ByteString`      |                                                             |
| XmlElement      | `UA_XmlElement`      | `UA_String` | `opcua::XmlElement`      |                                                             |
| NodeId          | `UA_NodeId`          |             | `opcua::NodeId`          |                                                             |
| ExpandedNodeId  | `UA_ExpandedNodeId`  |             | `opcua::ExpandedNodeId`  |                                                             |
| StatusCode      | `UA_StatusCode`      | `uint32_t`  | `opcua::StatusCode`      |                                                             |
| QualifiedName   | `UA_QualifiedName`   |             | `opcua::QualifiedName`   |                                                             |
| LocalizedText   | `UA_LocalizedText`   |             | `opcua::LocalizedText`   |                                                             |
| ExtensionObject | `UA_ExtensionObject` |             | `opcua::ExtensionObject` |                                                             |
| DataValue       | `UA_DataValue`       |             | `opcua::DataValue`       |                                                             |
| Variant         | `UA_Variant`         |             | `opcua::Variant`         |                                                             |
| DiagnosticInfo  | `UA_DiagnosticInfo`  |             | `opcua::DiagnosticInfo`  |                                                             |

## 🚀 Getting started

The library can be built, integrated and installed using [CMake](https://cmake.org/runningcmake/).

Please check out the open62541 build options here: https://www.open62541.org/doc/1.3/building.html#build-options

open62541++ provides additional build options:

- `UAPP_INTERNAL_OPEN62541`: Use internal open62541 library if `ON` or search for installed open62541 library if `OFF`
- `UAPP_BUILD_DOCUMENTATION`: Build documentation
- `UAPP_BUILD_EXAMPLES`: Build examples for `examples` directory
- `UAPP_BUILD_TESTS`: Build unit tests
- `UAPP_BUILD_TESTS_AUTORUN`: Run unit tests after build
- `UAPP_ENABLE_CLANG_TIDY`: Enable static code analysis with [Clang-Tidy](https://clang.llvm.org/extra/clang-tidy/)
- `UAPP_ENABLE_INCLUDE_WHAT_YOU_USE`: Enable static code analysis with [Include What You Use](https://github.com/include-what-you-use/include-what-you-use)
- `UAPP_ENABLE_COVERAGE`: Enable coverage analysis
- `UAPP_ENABLE_PCH`: Use precompiled headers to speed up compilation
- `UAPP_ENABLE_SANITIZER_ADDRESS/LEAK/MEMORY/THREAD/UNDEFINED_BEHAVIOUR`: Enable sanitizers

### Integrate as an embedded (in-source) dependency

Add it to your project as a Git submodule (`git submodule add https://github.com/open62541pp/open62541pp.git`) and link it with CMake:

```cmake
add_subdirectory(extern/open62541pp)  # the submodule directory
target_link_libraries(myexecutable PRIVATE open62541pp::open62541pp)
```

### Integrate as a pre-compiled library

If you build and install this package to your system, a `open62541ppConfig.cmake` file will be generated and installed to your system.
The installed library can be found and linked within CMake:

```cmake 
find_package(open62541pp::open62541pp CONFIG REQUIRED)
target_link_libraries(myexecutable PRIVATE open62541pp::open62541pp)
```

### Build and install

```shell
# clone repository
git clone --recursive https://github.com/open62541pp/open62541pp.git
cd open62541pp

# build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DUAPP_BUILD_EXAMPLES=ON -DUAPP_BUILD_TESTS=ON ..
cmake --build .

# run tests
ctest --output-on-failure

# generate coverage reports (text or html) with UAPP_ENABLE_COVERAGE option enabled
cmake --build . --target open62541pp_coverage_report
cmake --build . --target open62541pp_coverage_report_html

# install to system
cmake --install .
```

### Dependencies

- [open62541](https://github.com/open62541/open62541) as integrated submodule or external dependency
- [doctest](https://github.com/doctest/doctest) for tests as integrated submodule

## 🤝 Contribute

Contributions and feature requests are very welcome.
Please have a look at the [contribution guidelines](https://github.com/open62541pp/open62541pp/blob/master/CONTRIBUTING.md).
