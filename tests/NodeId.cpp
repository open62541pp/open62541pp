#include <catch2/catch_test_macros.hpp>

#include "open62541pp/NodeId.h"

using namespace opcua;

TEST_CASE("NodeId") {
    SECTION("Copy") {
        NodeId src(1, 0);
        NodeId dst(src);
        REQUIRE(dst == src);
    }

    SECTION("Assignment") {
        NodeId src(1, 0);
        NodeId dst(2, 1);
        dst = src;
        REQUIRE(dst == src);
    }

    SECTION("Namespace index") {
        REQUIRE(NodeId(1, 0).getNamespaceIndex() == 0);
        REQUIRE(NodeId(1, 2).getNamespaceIndex() == 2);
    }

    SECTION("Comparison") {
        REQUIRE(NodeId(1, 0) == NodeId(1, 0));
        REQUIRE(NodeId(1, 0) <= NodeId(1, 0));
        REQUIRE(NodeId(1, 0) >= NodeId(1, 0));
        REQUIRE(NodeId(1, 0) != NodeId(2, 0));
        REQUIRE(NodeId(1, 0) != NodeId(1, 1));
        REQUIRE(NodeId("a", 0) == NodeId("a", 0));
        REQUIRE(NodeId("a", 0) != NodeId("b", 0));
        REQUIRE(NodeId("a", 0) != NodeId("a", 1));

        // namespace index is compared before identifier
        REQUIRE(NodeId(1, 0) < NodeId(0, 1));
        REQUIRE(NodeId(1, 0) < NodeId(2, 0));

        REQUIRE(NodeId("a", 1) < NodeId("b", 1));
        REQUIRE(NodeId("b", 1) > NodeId("a", 1));
    }

    SECTION("Hash") {
        REQUIRE(NodeId(1, 0).hash() == NodeId(1, 0).hash());
        REQUIRE(NodeId(1, 0).hash() != NodeId(2, 0).hash());
        REQUIRE(NodeId(1, 0).hash() != NodeId(1, 1).hash());
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
