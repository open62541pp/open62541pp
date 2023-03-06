#include <chrono>
#include <thread>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "open62541pp/Helper.h"
#include "open62541pp/Node.h"
#include "open62541pp/NodeId.h"
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
}
