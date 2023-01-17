#include <catch2/catch_test_macros.hpp>

#include "open62541pp/NodeId.h"

using namespace opcua;

TEST_CASE("NodeId") {
    NodeId idNumeric1(1, 0);
    NodeId idNumeric2(1, 0);
    NodeId idNumeric3(2, 0);
    NodeId idNumeric4(2, 1);
    NodeId idString1("a", 0);
    NodeId idString2("a", 0);
    NodeId idString3("b", 0);
    NodeId idString4("b", 1);

    SECTION("Copy") {
        REQUIRE(NodeId(idNumeric1) == idNumeric1);
    }

    SECTION("Assignment") {
        NodeId tmp(0, 0);
        tmp = idNumeric1;
        REQUIRE(tmp == idNumeric1);
    }

    SECTION("Namespace index") {
        REQUIRE(idNumeric1.getNamespaceIndex() == 0);
        REQUIRE(idNumeric2.getNamespaceIndex() == 0);
        REQUIRE(idNumeric3.getNamespaceIndex() == 0);
        REQUIRE(idNumeric4.getNamespaceIndex() == 1);
        REQUIRE(idString1.getNamespaceIndex() == 0);
        REQUIRE(idString2.getNamespaceIndex() == 0);
        REQUIRE(idString3.getNamespaceIndex() == 0);
        REQUIRE(idString4.getNamespaceIndex() == 1);
    }

    SECTION("Comparison") {
        REQUIRE(idNumeric1 == idNumeric2);
        REQUIRE(idNumeric1 != idNumeric3);
        REQUIRE(idNumeric3 != idNumeric4);
        REQUIRE(idString1 != idNumeric1);
        REQUIRE(idString1 == idString2);
        REQUIRE(idString2 != idString3);
        REQUIRE(idString3 != idString4);
    }

    SECTION("Hash") {
        REQUIRE(idNumeric1.hash() == idNumeric2.hash());
        REQUIRE(idNumeric1.hash() != idNumeric3.hash());
        REQUIRE(idNumeric3.hash() != idNumeric4.hash());
        REQUIRE(idString1.hash() != idNumeric1.hash());
        REQUIRE(idString1.hash() == idString2.hash());
        REQUIRE(idString2.hash() != idString3.hash());
        REQUIRE(idString3.hash() != idString4.hash());
    }

    SECTION("Get properties (getIdentifierType, getNamespaceIndex, getIdentifier") {
        {
            NodeId id(UA_NODEID_NUMERIC(1, 111));
            REQUIRE(id.getIdentifierType() == NodeIdType::Numeric);
            REQUIRE(id.getNamespaceIndex() == 1);
            REQUIRE(id.getIdentifierAs<NodeIdType::Numeric>() == 111);
        }
        {
            NodeId id(UA_NODEID_STRING_ALLOC(2, "Test123"));
            REQUIRE(id.getIdentifierType() == NodeIdType::String);
            REQUIRE(id.getNamespaceIndex() == 2);
            REQUIRE(std::get<String>(id.getIdentifier()).get() == "Test123");
        }
        {
            Guid guid(11, 22, 33, {1, 2, 3, 4, 5, 6, 7, 8});
            NodeId id(guid, 3);
            REQUIRE(id.getIdentifierType() == NodeIdType::Guid);
            REQUIRE(id.getNamespaceIndex() == 3);
            REQUIRE(id.getIdentifierAs<Guid>() == guid);
        }
        {
            ByteString byteString("Test456");
            NodeId id(byteString, 4);
            REQUIRE(id.getIdentifierType() == NodeIdType::ByteString);
            REQUIRE(id.getNamespaceIndex() == 4);
            REQUIRE(id.getIdentifierAs<ByteString>() == byteString);
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
