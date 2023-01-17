#include <catch2/catch_test_macros.hpp>

#include "open62541pp/NodeId.h"

using namespace opcua;

TEST_CASE("NodeId") {
    NodeId nodeNumeric1(1, 0);
    NodeId nodeNumeric2(1, 0);
    NodeId nodeNumeric3(2, 0);
    NodeId nodeNumeric4(2, 1);
    NodeId nodeString1("a", 0);
    NodeId nodeString2("a", 0);
    NodeId nodeString3("b", 0);
    NodeId nodeString4("b", 1);

    SECTION("Copy") {
        REQUIRE(NodeId(nodeNumeric1) == nodeNumeric1);
    }

    SECTION("Assignment") {
        NodeId tmp(0, 0);
        tmp = nodeNumeric1;
        REQUIRE(tmp == nodeNumeric1);
    }

    SECTION("Namespace index") {
        REQUIRE(nodeNumeric1.getNamespaceIndex() == 0);
        REQUIRE(nodeNumeric2.getNamespaceIndex() == 0);
        REQUIRE(nodeNumeric3.getNamespaceIndex() == 0);
        REQUIRE(nodeNumeric4.getNamespaceIndex() == 1);
        REQUIRE(nodeString1.getNamespaceIndex() == 0);
        REQUIRE(nodeString2.getNamespaceIndex() == 0);
        REQUIRE(nodeString3.getNamespaceIndex() == 0);
        REQUIRE(nodeString4.getNamespaceIndex() == 1);
    }

    SECTION("Comparison") {
        REQUIRE(nodeNumeric1 == nodeNumeric2);
        REQUIRE(nodeNumeric1 != nodeNumeric3);
        REQUIRE(nodeNumeric3 != nodeNumeric4);
        REQUIRE(nodeString1 != nodeNumeric1);
        REQUIRE(nodeString1 == nodeString2);
        REQUIRE(nodeString2 != nodeString3);
        REQUIRE(nodeString3 != nodeString4);
    }

    SECTION("Hash") {
        REQUIRE(nodeNumeric1.hash() == nodeNumeric2.hash());
        REQUIRE(nodeNumeric1.hash() != nodeNumeric3.hash());
        REQUIRE(nodeNumeric3.hash() != nodeNumeric4.hash());
        REQUIRE(nodeString1.hash() != nodeNumeric1.hash());
        REQUIRE(nodeString1.hash() == nodeString2.hash());
        REQUIRE(nodeString2.hash() != nodeString3.hash());
        REQUIRE(nodeString3.hash() != nodeString4.hash());
    }

    SECTION("Get properties (getIdentifierType, getNamespaceInex, getIdentifier") {
        {
            NodeId id(UA_NODEID_NUMERIC(1, 111));
            REQUIRE(id.getIdentifierType() == UA_NODEIDTYPE_NUMERIC);
            REQUIRE(id.getNamespaceIndex() == 1);
            REQUIRE(std::get<0>(id.getIdentifier()) == 111);
        }
        {
            NodeId id(UA_NODEID_STRING_ALLOC(2, "Test123"));
            REQUIRE(id.getIdentifierType() == UA_NODEIDTYPE_STRING);
            REQUIRE(id.getNamespaceIndex() == 2);
            REQUIRE(std::get<String>(id.getIdentifier()).get() == "Test123");
        }
        {
            Guid guid(11, 22, 33, {1, 2, 3, 4, 5, 6, 7, 8});
            NodeId id(guid, 3);
            REQUIRE(id.getIdentifierType() == UA_NODEIDTYPE_GUID);
            REQUIRE(id.getNamespaceIndex() == 3);
            REQUIRE(std::get<Guid>(id.getIdentifier()) == guid);
        }
        {
            ByteString byteString("Test456");
            NodeId id(byteString, 4);
            REQUIRE(id.getIdentifierType() == UA_NODEIDTYPE_BYTESTRING);
            REQUIRE(id.getNamespaceIndex() == 4);
            REQUIRE(std::get<ByteString>(id.getIdentifier()) == byteString);
        }
    }
}

TEST_CASE("ExpandedNodeId") {
    ExpandedNodeId idLocal({"local", 1}, {}, 0);
    REQUIRE(idLocal.isLocal());

    ExpandedNodeId idFull({"full", 1}, "namespace", 1);
    REQUIRE(idFull.getNodeId() == NodeId{"full", 1});
    REQUIRE(idFull.getNamespaceUri() == "namespace");
    REQUIRE(idFull.getServerIndex() == 1);

    REQUIRE(idLocal == idLocal);
    REQUIRE(idLocal != idFull);
}
