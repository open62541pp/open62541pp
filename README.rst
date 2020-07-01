open62541++
===========

|license badge| |travis badge| |appveyor badge| |coveralls badge| |rtd badge|

.. |license badge| image:: https://img.shields.io/badge/License-MPL%202.0-blue.svg
    :target: https://github.com/open62541pp/open62541pp/blob/master/LICENSE
    :alt: License: MPL 2.0

.. |travis badge| image:: https://travis-ci.org/open62541pp/open62541pp.svg?branch=master
    :target: https://travis-ci.org/open62541pp/open62541pp
    :alt: Build Status

.. |appveyor badge| image:: https://ci.appveyor.com/api/projects/status/802vyxytdii4tc3v/branch/master?svg=true
    :target: https://ci.appveyor.com/project/lukasberbuer/open62541pp/branch/master
    :alt: Build Status

.. |coveralls badge| image:: https://coveralls.io/repos/github/open62541pp/open62541pp/badge.svg
    :target: https://coveralls.io/github/open62541pp/open62541pp
    :alt: Coverage Status

.. |rtd badge| image:: https://readthedocs.org/projects/open62541pp/badge/?version=latest
    :target: https://open62541pp.readthedocs.io/en/latest/?badge=latest
    :alt: Documentation Status

open62541++ is a C++ wrapper built on top of the amazing `open62541 <https://open62541.org>`__ OPC UA (OPC Unified Architecture) library.

The documentation is available on https://open62541pp.readthedocs.io/en/latest/.

It aims to:

- safely wrap the open62541 UA_* types to prevent memory leaks.
- expose high level and easy to use *Server*, *Client* (TODO), and *Node* classes similar to the `python-opcua API <https://python-opcua.readthedocs.io/en/latest/index.html>`__
- minimize code
- reduce the hurdle to get started with OPC UA
- use modern C++ (C++ 17) and best practices
- native open62541 objects can be accessed using the `handle()` method of the wrapping classes to give you all the power of open62541 (the open62541++ API is quite limited at the moment)

Dependencies
------------

- `open62541 <https://github.com/open62541/open62541>`__ (included as submodule)
- `catch2 <https://github.com/catchorg/Catch2>`__ (included)

Build
-----

The library is built using `CMake <https://cmake.org/runningcmake/>`__. Please check out the open62541 build options here: https://open62541.org/doc/1.0/building.html

Example
-------

.. code-block:: cpp

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

