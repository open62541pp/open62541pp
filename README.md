# open62541++

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-blue.svg)](https://github.com/open62541pp/open62541pp/blob/master/LICENSE)
[![CI](https://github.com/open62541pp/open62541pp/actions/workflows/ci.yml/badge.svg)](https://github.com/open62541pp/open62541pp/actions/workflows/ci.yml)
[![Documentation](https://github.com/open62541pp/open62541pp/actions/workflows/doc.yml/badge.svg)](https://github.com/open62541pp/open62541pp/actions/workflows/doc.yml)
[![Compatibility](https://github.com/open62541pp/open62541pp/actions/workflows/open62541-compatibility.yml/badge.svg)](https://github.com/open62541pp/open62541pp/actions/workflows/open62541-compatibility.yml)
[![Coverage](https://coveralls.io/repos/github/open62541pp/open62541pp/badge.svg)](https://coveralls.io/github/open62541pp/open62541pp)

**[Documentation](https://open62541pp.github.io/open62541pp) · [Examples](https://github.com/open62541pp/open62541pp/tree/master/examples)**

open62541++ is a C++ wrapper built on top of the amazing [open62541](https://open62541.org) OPC UA (OPC Unified Architecture) library.

Features and goals:

- High-level and easy to use classes similar to the [python-opcua API](https://python-opcua.readthedocs.io):
  - `opcua::Server`
  - `opcua::Client` *TODO*
  - `opcua::Node`
- Safe wrapper classes for open62541 `UA_*` types to prevent memory leaks
- Native open62541 objects can be accessed using the `handle()` method of the wrapping classes
- Extensible type conversion system to convert from arbitrary types to native `UA_*` types
- Less hurdle to get started with OPC UA
- Use modern C++ (C++ 17) and best practices

## Example

```cpp
#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server;

    const opcua::NodeId myIntegerNodeId{"the.answer", 1};
    const std::string   myIntegerName{"the answer"};

    // add variable node
    auto parentNode    = server.getObjectsNode();
    auto myIntegerNode = parentNode.addVariable(myIntegerNodeId, myIntegerName, opcua::Type::Int32);

    // set node attributes
    myIntegerNode.setDisplayName("the answer", "en-US");
    myIntegerNode.setDescription("the answer", "en-US");

    // write value
    myIntegerNode.write(42);

    // read value
    std::cout << "The answer is: " << myIntegerNode.read<int>() << std::endl;

    server.run();
}
```

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

## Build

The library is built using [CMake](https://cmake.org/runningcmake/). You can add it to your project with as Git submodule (`git submodule add https://github.com/open62541pp/open62541pp.git`) and link it with CMake:

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
