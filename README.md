# open62541++

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-blue.svg)](https://github.com/open62541pp/open62541pp/blob/master/LICENSE)
[![Build Status](https://travis-ci.org/open62541pp/open62541pp.svg?branch=master)](https://travis-ci.org/open62541pp/open62541pp)
[![Coverage Status](https://coveralls.io/repos/github/open62541pp/open62541pp/badge.svg)](https://coveralls.io/github/open62541pp/open62541pp)

open62541++ is a C++ wrapper built on top of the amazing [open62541](https://open62541.org) OPC UA (OPC Unified Architecture) library.

It aims to:
- safely wrap the open62541 UA_* types to prevent memory leaks.
- expose high level and easy to use *Server*, *Client* (TODO), and *Node* classes similar to the [python-opcua API](https://python-opcua.readthedocs.io/en/latest/index.html)
- minimize code
- reduce the hurdle to get started with OPC UA
- use modern C++ (C++ 17) and best practices
- native open62541 objects can be accessed using the `handle()` method of the wrapping classes to give you all the power of open62541 (the open62541++ API is quite limited at the moment)

## Dependencies

- [open62541](https://github.com/open62541/open62541) (included as submodule)
- [catch2](https://github.com/catchorg/Catch2) (included)

## Build

The library is built using [CMake](https://cmake.org/runningcmake/). Please check out the open62541 build options here: https://open62541.org/doc/1.0/building.html

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