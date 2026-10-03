#include <catch2/catch_test_macros.hpp>

#include "open62541pp/config.hpp"
#include "open62541pp/plugin/historydatabase.hpp"
#include "open62541pp/server.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"
#include "open62541pp/services/nodemanagement.hpp"

using namespace opcua;

#if UAPP_HAS_HISTORIZING

TEST_CASE("History database") {
    Server server;

    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};
    const NodeId variableId{1, 1000};

    const auto addVariable = [&] {
        REQUIRE(services::addVariable(
            server,
            objectsId,
            variableId,
            "Historized",
            VariableAttributes{}
                .setAccessLevel(
                    AccessLevel::CurrentRead | AccessLevel::CurrentWrite | AccessLevel::HistoryRead
                )
                .setDataType<int>()
                .setValue(Variant{0}),
            VariableTypeId::BaseDataVariableType,
            ReferenceTypeId::HasComponent
        ));
    };

    SECTION("historizeNode without history database") {
        addVariable();
        CHECK(historizeNode(server, variableId) == UA_STATUSCODE_BADCONFIGURATIONERROR);
    }

    SECTION("useHistoryDatabase") {
        CHECK(server.config()->historyDatabase.context == nullptr);
        useHistoryDatabase(server);
        CHECK(server.config()->historyDatabase.context != nullptr);
    }

    SECTION("useHistoryDatabase is idempotent") {
        useHistoryDatabase(server);
        const auto* context = server.config()->historyDatabase.context;
        useHistoryDatabase(server);
        CHECK(server.config()->historyDatabase.context == context);
    }

    SECTION("historizeNode") {
        useHistoryDatabase(server);
        addVariable();
        CHECK_FALSE(services::readHistorizing(server, variableId).value());
        CHECK(historizeNode(server, variableId).isGood());
        CHECK(services::readHistorizing(server, variableId).value());
    }

    SECTION("historizeNode with custom settings") {
        useHistoryDatabase(server);
        addVariable();
        HistorizingSettings settings{};
        settings.maxResponseSize = 10;
        settings.updateStrategy = HistorizingUpdateStrategy::Poll;
        settings.pollingInterval = 100;
        settings.capacity = 10;
        CHECK(historizeNode(server, variableId, settings).isGood());
    }

    SECTION("historizeNode with unknown node") {
        useHistoryDatabase(server);
        CHECK(historizeNode(server, {1, 12345}).isBad());
    }

    SECTION("historize multiple nodes") {
        useHistoryDatabase(server, 3);
        for (uint32_t i = 0; i < 3; ++i) {
            const NodeId id{1, 2000 + i};
            REQUIRE(services::addVariable(
                server,
                objectsId,
                id,
                "Historized",
                VariableAttributes{}
                    .setAccessLevel(AccessLevel::CurrentRead | AccessLevel::HistoryRead)
                    .setDataType<int>()
                    .setValue(Variant{0}),
                VariableTypeId::BaseDataVariableType,
                ReferenceTypeId::HasComponent
            ));
            CHECK(historizeNode(server, id).isGood());
        }
    }
}

#endif
