# open62541++

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-blue.svg)](https://github.com/open62541pp/open62541pp/blob/master/LICENSE)
[![CI](https://github.com/open62541pp/open62541pp/actions/workflows/ci.yml/badge.svg)](https://github.com/open62541pp/open62541pp/actions/workflows/ci.yml)
[![Coverage](https://coveralls.io/repos/github/open62541pp/open62541pp/badge.svg)](https://coveralls.io/github/open62541pp/open62541pp)
[![Documentation](https://readthedocs.org/projects/open62541pp/badge/?version=latest)](https://open62541pp.readthedocs.io/en/latest/?badge=latest)

open62541++ is a C++ wrapper built on top of the amazing [open62541](https://open62541.org) OPC UA (OPC Unified Architecture) library.

The documentation is available on https://open62541pp.readthedocs.io/en/latest/.

It aims to:

- safely wrap the open62541 UA_* types to prevent memory leaks.
- expose high level and easy to use *Server*, *Client* (TODO), and *Node* classes similar to the [python-opcua API](https://python-opcua.readthedocs.io/en/latest/index.html)
- minimize code
- reduce the hurdle to get started with OPC UA
- use modern C++ (C++ 17) and best practices
- native open62541 objects can be accessed using the `handle()` method of the wrapping classes to give you all the power of open62541 (the open62541++ API is quite limited at the moment)

## Example

```cpp
#include <iostream>

#include "open62541pp/open62541pp.h"

int main() {
    opcua::Server server;

    const auto        myIntegerNodeId = opcua::NodeId("the.answer", 1);
    const std::string myIntegerName   = "the answer";

    // create node
    auto parentNode    = server.getObjectsNode();
    auto myIntegerNode = parentNode.addVariable(myIntegerNodeId, myIntegerName, opcua::Type::Int32);

    // set node attributes
    myIntegerNode.setDisplayName("the answer");
    myIntegerNode.setDescription("the answer");

    // write value
    myIntegerNode.write(42);

    // read value
    std::cout << "The answer is: " << myIntegerNode.read<int>() << std::endl;

    server.start();

    return 0;
}
```

## Build

The library is built using [CMake](https://cmake.org/runningcmake/). You can add it to your project with as Git submodule (`git submodule add https://github.com/open62541pp/open62541pp.git`) and link it with CMake:

```cmake
add_subdirectory(extern/open62541pp)  # the submodule directory
target_link_library(myexecutable PRIVATE open62541pp::open62541pp)
```

Please check out the open62541 build options here: https://open62541.org/doc/current/building.html

open62541++ provides additional build options:

- `UAPP_INTERNAL_OPEN62541`: Use internal open62541 library if `ON` or search for installed open62541 library if `OFF`
- `UAPP_BUILD_EXAMPLES`: Build examples for `examples` directory
- `UAPP_BUILD_TESTS`: Build an run tests
- `UAPP_ENABLE_CLANG_TIDY`: Enable static code analysis with Clang-Tidy
- `UAPP_ENABLE_COVERAGE`: Enable coverage analysis
- `UAPP_ENABLE_SANITIZER_ADDRESS/LEAK/MEMORY/THREAD/UNDEFINED_BEHAVIOUR`: Enable sanitizers

### Dependencies

- [open62541](https://github.com/open62541/open62541) as integrated submodule or external dependency
- [catch2](https://github.com/catchorg/Catch2) for tests
