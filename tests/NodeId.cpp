#include "catch2/catch.hpp"

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
        NodeId tmp(nodeNumeric1); // NOLINT
        REQUIRE(tmp == nodeNumeric1);
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
        REQUIRE(nodeString1.getNamespaceIndex()  == 0);
        REQUIRE(nodeString2.getNamespaceIndex()  == 0);
        REQUIRE(nodeString3.getNamespaceIndex()  == 0);
        REQUIRE(nodeString4.getNamespaceIndex()  == 1);
    }

    SECTION("Comparison") {
        REQUIRE(nodeNumeric1 == nodeNumeric2);
        REQUIRE(nodeNumeric1 != nodeNumeric3);
        REQUIRE(nodeNumeric3 != nodeNumeric4);
        REQUIRE(nodeString1  != nodeNumeric1);
        REQUIRE(nodeString1  == nodeString2);
        REQUIRE(nodeString2  != nodeString3);
        REQUIRE(nodeString3  != nodeString4);
    }

    SECTION("Hash") {
        REQUIRE(nodeNumeric1.hash() == nodeNumeric2.hash());
        REQUIRE(nodeNumeric1.hash() != nodeNumeric3.hash());
        REQUIRE(nodeNumeric3.hash() != nodeNumeric4.hash());
        REQUIRE(nodeString1.hash()  != nodeNumeric1.hash());
        REQUIRE(nodeString1.hash()  == nodeString2.hash());
        REQUIRE(nodeString2.hash()  != nodeString3.hash());
        REQUIRE(nodeString3.hash()  != nodeString4.hash());
    }
}