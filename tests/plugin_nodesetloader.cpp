#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/config.hpp"
#include "open62541pp/plugin/nodesetloader.hpp"
#include "open62541pp/server.hpp"

#if UAPP_HAS_NODESETLOADER

namespace fs = std::filesystem;
using namespace opcua;

TEST_CASE("loadNodeset") {
    Server server;
    REQUIRE(server.namespaceArray().size() == 2);

    SECTION("Invalid file") {
        REQUIRE(loadNodeset(server, "invalid.xml").isBad());
    }

    SECTION("DI") {
        const auto file = fs::path{UAPP_NODESET_DIR} / "DI" / "Opc.Ua.Di.NodeSet2.xml";
        CAPTURE(file);
        REQUIRE(fs::exists(file));
        REQUIRE(loadNodeset(server, file.c_str()).isGood());

        const auto ns = server.namespaceArray();
        CHECK(ns.size() == 3);
        CHECK(ns.back() == "http://opcfoundation.org/UA/DI/");
    }
}

#endif
