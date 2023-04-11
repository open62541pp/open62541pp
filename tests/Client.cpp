#include <chrono>
#include <thread>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "open62541pp/Helper.h"
#include "open62541pp/Node.h"
#include "open62541pp/types/NodeId.h"
#include "open62541pp/Server.h"
#include "open62541pp/Client.h"

#include "open62541_impl.h"

using namespace Catch::Matchers;
using namespace std::chrono_literals;
using namespace opcua;

static bool compareNodes(NodeId id, uint16_t numericId) {
	auto uaNode = UA_NODEID_NUMERIC(0, numericId);
	return UA_NodeId_equal(id.handle(), &uaNode);
}

TEST_CASE("Client") {
	Server server;

	REQUIRE_FALSE(server.isRunning());

	auto t = std::thread([&] { server.run(); });

	std::this_thread::sleep_for(100ms);  // wait for thread to execute run method
	
	REQUIRE(server.isRunning());
	REQUIRE_NOTHROW(server.stop());

	server.setCustomHostname("customhost");

	server.setApplicationName("Test App");

	server.setApplicationUri("http://app.com");

	server.setProductUri("http://product.com");

	SECTION("List Endpoints")
	{
		Client client;
		auto servers = client.findServers("opc.tcp://localhost:4840");		
		REQUIRE(servers.size() >= 1);
	}
        SECTION("Get Endpoints")
        {
                Client client;
                auto endpoints = client.getEndpoints("opc.tcp://localhost:4840");
                REQUIRE(endpoints.size() >= 1);
        }

        SECTION("Client::connect")
        {
                Client client;
                REQUIRE_NOTHROW(client.connect("opc.tcp://localhost:4840"));
        }

        SECTION("Client::readValueAttributes")
        {
                Client client;
                REQUIRE_NOTHROW(client.connect("opc.tcp://localhost:4840"));
                    // clang-format off
        auto rootFolder = client.readValueAttribute(NodeId{0, UA_NS0ID_ROOTFOLDER});
        auto objFolder = client.readValueAttribute(NodeId{0, UA_NS0ID_OBJECTSFOLDER});
        auto typesFolder = client.readValueAttribute(NodeId{0, UA_NS0ID_TYPESFOLDER});
        auto viewsFolder = client.readValueAttribute(NodeId{0, UA_NS0ID_VIEWSFOLDER});
        auto objTypesFolder = client.readValueAttribute(NodeId{0, UA_NS0ID_OBJECTTYPESFOLDER});
        auto varTypesFolder = client.readValueAttribute(NodeId{0, UA_NS0ID_VARIABLETYPESFOLDER});
        auto dataTypesFolder = client.readValueAttribute(NodeId{0, UA_NS0ID_DATATYPESFOLDER});
        auto refTypesFolder = client.readValueAttribute(NodeId{0, UA_NS0ID_REFERENCETYPESFOLDER});
        auto baseObjType = client.readValueAttribute(NodeId{0, UA_NS0ID_BASEOBJECTTYPE});
        auto baseDataVariableType = client.readValueAttribute(NodeId{0, UA_NS0ID_BASEDATAVARIABLETYPE});
        }

}
