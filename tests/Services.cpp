#include <functional>  // reference_wrapper
#include <utility>  // pair
#include <variant>
#include <vector>

#include <doctest/doctest.h>

#include "open62541pp/Client.h"
#include "open62541pp/Server.h"
#include "open62541pp/services/services.h"

#include "helper/Runner.h"

using namespace opcua;

TEST_CASE("NodeManagement (server)") {
    Server server;
    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    SUBCASE("Non-type nodes") {
        services::addFolder(server, objectsId, {1, 1000}, "folder");
        services::addObject(server, objectsId, {1, 1001}, "object");
        services::addVariable(server, objectsId, {1, 1002}, "variable");
        services::addProperty(server, objectsId, {1, 1003}, "property");
    }

    SUBCASE("Type nodes") {
        services::addObjectType(server, {0, UA_NS0ID_BASEOBJECTTYPE}, {1, 1000}, "objecttype");
        services::addVariableType(
            server, {0, UA_NS0ID_BASEVARIABLETYPE}, {1, 1001}, "variabletype"
        );
    }

    SUBCASE("Add existing node id") {
        services::addObject(server, objectsId, {1, 1000}, "object1");
        CHECK_THROWS_WITH(
            services::addObject(server, objectsId, {1, 1000}, "object2"), "BadNodeIdExists"
        );
    }

    SUBCASE("Add reference") {
        services::addFolder(server, objectsId, {1, 1000}, "folder");
        services::addObject(server, objectsId, {1, 1001}, "object");
        services::addReference(server, {1, 1000}, {1, 1001}, ReferenceType::Organizes);
        CHECK_THROWS_WITH(
            services::addReference(server, {1, 1000}, {1, 1001}, ReferenceType::Organizes),
            "BadDuplicateReferenceNotAllowed"
        );
    }

    SUBCASE("Delete node") {
        services::addObject(server, objectsId, {1, 1000}, "object");
        services::deleteNode(server, {1, 1000});
        CHECK_THROWS_WITH(services::deleteNode(server, {1, 1000}), "BadNodeIdUnknown");
    }
}

TEST_CASE("NodeManagement (client)") {
    Server server;
    ServerRunner serverRunner(server);

    Client client;
    client.connect("opc.tcp://localhost:4840");

    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    SUBCASE("Non-type nodes") {
        CHECK_NOTHROW(services::addObject(client, objectsId, {1, 1000}, "object"));
        CHECK(services::readNodeClass(server, {1, 1000}) == NodeClass::Object);

        CHECK_NOTHROW(services::addFolder(client, objectsId, {1, 1001}, "folder"));
        CHECK(services::readNodeClass(server, {1, 1001}) == NodeClass::Object);

        CHECK_NOTHROW(services::addVariable(client, objectsId, {1, 1002}, "variable"));
        CHECK(services::readNodeClass(server, {1, 1002}) == NodeClass::Variable);

        CHECK_NOTHROW(services::addProperty(client, objectsId, {1, 1003}, "property"));
        CHECK(services::readNodeClass(server, {1, 1003}) == NodeClass::Variable);
    }

    SUBCASE("Type nodes") {
        CHECK_NOTHROW(
            services::addObjectType(client, {0, UA_NS0ID_BASEOBJECTTYPE}, {1, 1000}, "objecttype")
        );
        CHECK(services::readNodeClass(server, {1, 1000}) == NodeClass::ObjectType);

        CHECK_NOTHROW(services::addVariableType(
            client, {0, UA_NS0ID_BASEVARIABLETYPE}, {1, 1001}, "variabletype"
        ));
        CHECK(services::readNodeClass(server, {1, 1001}) == NodeClass::VariableType);
    }

    SUBCASE("Add reference") {
        services::addFolder(client, objectsId, {1, 1000}, "folder");
        services::addObject(client, objectsId, {1, 1001}, "object");
        services::addReference(client, {1, 1000}, {1, 1001}, ReferenceType::Organizes);
        CHECK_THROWS_WITH(
            services::addReference(client, {1, 1000}, {1, 1001}, ReferenceType::Organizes),
            "BadDuplicateReferenceNotAllowed"
        );
    }

    SUBCASE("Delete node") {
        services::addObject(client, objectsId, {1, 1000}, "object");
        services::deleteNode(client, {1, 1000});
        CHECK_THROWS_WITH(services::deleteNode(client, {1, 1000}), "BadNodeIdUnknown");
    }
}

TEST_CASE("Attribute (server)") {
    Server server;
    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    SUBCASE("Read/write node attributes") {
        const NodeId id{1, "testAttributes"};
        services::addVariable(server, objectsId, id, "testAttributes");

        // read default attributes
        CHECK(services::readNodeId(server, id) == id);
        CHECK(services::readNodeClass(server, id) == NodeClass::Variable);
        CHECK(services::readBrowseName(server, id) == "testAttributes");
        //_EQ CHECK(services::readDisplayName(server, id), LocalizedText("", "testAttributes"));
        CHECK(services::readDescription(server, id).getText().empty());
        CHECK(services::readDescription(server, id).getLocale().empty());
        CHECK(services::readWriteMask(server, id) == 0);
        const uint32_t adminUserWriteMask = ~0;  // all bits set
        CHECK(services::readUserWriteMask(server, id) == adminUserWriteMask);
        CHECK(services::readDataType(server, id) == NodeId(0, UA_NS0ID_BASEDATATYPE));
        CHECK(services::readValueRank(server, id) == ValueRank::Any);
        CHECK(services::readArrayDimensions(server, id).empty());
        CHECK(services::readAccessLevel(server, id) == UA_ACCESSLEVELMASK_READ);
        const uint8_t adminUserAccessLevel = ~0;  // all bits set
        CHECK(services::readUserAccessLevel(server, id) == adminUserAccessLevel);
        CHECK(services::readMinimumSamplingInterval(server, id) == 0.0);

        // write new attributes
        CHECK_NOTHROW(services::writeDisplayName(server, id, {"en-US", "newDisplayName"}));
        CHECK_NOTHROW(services::writeDescription(server, id, {"de-DE", "newDescription"}));
        CHECK_NOTHROW(services::writeWriteMask(server, id, 11));
        CHECK_NOTHROW(services::writeDataType(server, id, NodeId{0, 2}));
        CHECK_NOTHROW(services::writeValueRank(server, id, ValueRank::TwoDimensions));
        CHECK_NOTHROW(services::writeArrayDimensions(server, id, {3, 2}));
        const uint8_t newAccessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
        CHECK_NOTHROW(services::writeAccessLevel(server, id, newAccessLevel));
        CHECK_NOTHROW(services::writeMinimumSamplingInterval(server, id, 10.0));

        // read new attributes
        CHECK(services::readDisplayName(server, id) == LocalizedText("en-US", "newDisplayName"));
        CHECK(services::readDescription(server, id) == LocalizedText("de-DE", "newDescription"));
        CHECK(services::readWriteMask(server, id) == 11);
        CHECK(services::readDataType(server, id) == NodeId(0, 2));
        CHECK(services::readValueRank(server, id) == ValueRank::TwoDimensions);
        CHECK(services::readArrayDimensions(server, id).size() == 2);
        CHECK(services::readArrayDimensions(server, id).at(0) == 3);
        CHECK(services::readArrayDimensions(server, id).at(1) == 2);
        CHECK(services::readAccessLevel(server, id) == newAccessLevel);
        CHECK(services::readMinimumSamplingInterval(server, id) == 10.0);
    }

    SUBCASE("Read/write reference node attributes") {
        const NodeId id{0, UA_NS0ID_REFERENCES};

        // read default attributes
        CHECK(services::readIsAbstract(server, id) == true);
        CHECK(services::readSymmetric(server, id) == true);
        CHECK(services::readInverseName(server, id) == LocalizedText("", "References"));

        // write new attributes
        CHECK_NOTHROW(services::writeIsAbstract(server, id, false));
        CHECK_NOTHROW(services::writeSymmetric(server, id, false));
        CHECK_NOTHROW(services::writeInverseName(server, id, LocalizedText("", "New")));

        // read new attributes
        CHECK(services::readIsAbstract(server, id) == false);
        CHECK(services::readSymmetric(server, id) == false);
        CHECK(services::readInverseName(server, id) == LocalizedText("", "New"));
    }

    SUBCASE("Value rank and array dimension combinations") {
        const NodeId id{1, "testDimensions"};
        services::addVariable(server, objectsId, id, "testDimensions");

        SUBCASE("Unspecified dimension (ValueRank <= 0)") {
            const std::vector<ValueRank> valueRanks = {
                ValueRank::Any,
                ValueRank::Scalar,
                ValueRank::ScalarOrOneDimension,
                ValueRank::OneOrMoreDimensions,
            };
            for (auto valueRank : valueRanks) {
                CAPTURE(valueRank);
                services::writeValueRank(server, id, valueRank);
                CHECK_NOTHROW(services::writeArrayDimensions(server, id, {}));
                CHECK_THROWS(services::writeArrayDimensions(server, id, {1}));
                CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2}));
                CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2, 3}));
            }
        }

        SUBCASE("OneDimension") {
            services::writeValueRank(server, id, ValueRank::OneDimension);
            services::writeArrayDimensions(server, id, {1});
            CHECK_THROWS(services::writeArrayDimensions(server, id, {}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2, 3}));
        }

        SUBCASE("TwoDimensions") {
            services::writeValueRank(server, id, ValueRank::TwoDimensions);
            services::writeArrayDimensions(server, id, {1, 2});
            CHECK_THROWS(services::writeArrayDimensions(server, id, {}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2, 3}));
        }

        SUBCASE("ThreeDimensions") {
            services::writeValueRank(server, id, ValueRank::ThreeDimensions);
            services::writeArrayDimensions(server, id, {1, 2, 3});
            CHECK_THROWS(services::writeArrayDimensions(server, id, {}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1}));
            CHECK_THROWS(services::writeArrayDimensions(server, id, {1, 2}));
        }
    }

    SUBCASE("Read/write value") {
        const NodeId id{1, "testValue"};
        services::addVariable(server, objectsId, id, "testValue");

        Variant variantWrite;
        variantWrite.setScalarCopy(11.11);
        services::writeValue(server, id, variantWrite);

        Variant variantRead;
        services::readValue(server, id, variantRead);
        CHECK(variantRead.getScalar<double>() == 11.11);
    }

    SUBCASE("Read/write data value") {
        const NodeId id{1, "testDataValue"};
        services::addVariable(server, objectsId, id, "testDataValue");

        Variant variant;
        variant.setScalarCopy<int>(11);
        DataValue valueWrite(variant, {}, DateTime::now(), {}, 1, UA_STATUSCODE_GOOD);
        services::writeDataValue(server, id, valueWrite);

        DataValue valueRead;
        services::readDataValue(server, id, valueRead);

        CHECK_EQ(valueRead->hasValue, true);
        CHECK_EQ(valueRead->hasServerTimestamp, true);
        CHECK_EQ(valueRead->hasSourceTimestamp, true);
        CHECK_EQ(valueRead->hasServerPicoseconds, true);
        CHECK_EQ(valueRead->hasSourcePicoseconds, true);
        CHECK_EQ(valueRead->hasStatus, false);  // doesn't contain error code on success

        CHECK(valueRead.getValue().value().getScalar<int>() == 11);
        CHECK(valueRead->sourceTimestamp == valueWrite->sourceTimestamp);
        CHECK(valueRead->sourcePicoseconds == valueWrite->sourcePicoseconds);
    }
}

TEST_CASE("Attribute (server & client)") {
    Server server;
    ServerRunner serverRunner(server);

    Client client;
    client.connect("opc.tcp://localhost:4840");

    // create variable node
    const NodeId id{1, 1000};
    services::addVariable(server, {0, UA_NS0ID_OBJECTSFOLDER}, id, "variable");
    services::writeAccessLevel(server, id, UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE);
    services::writeWriteMask(server, id, ~0U);  // set all bits to 1 -> allow all

    // check read-only attributes
    CHECK(services::readNodeId(server, id) == services::readNodeId(client, id));
    CHECK(services::readNodeClass(server, id) == services::readNodeClass(client, id));
    CHECK(services::readBrowseName(server, id) == services::readBrowseName(client, id));

    // generate test matrix of possible reader/writer combinations
    // https://github.com/doctest/doctest/blob/v2.4.11/doc/markdown/parameterized-tests.md
    using ServerRef = std::reference_wrapper<Server>;
    using ClientRef = std::reference_wrapper<Client>;
    using ServerOrClientRef = std::variant<ServerRef, ClientRef>;

    ServerOrClientRef writerVar{server};
    ServerOrClientRef readerVar{server};
    // clang-format off
    SUBCASE("server/server") { writerVar = server; readerVar = server; }
    SUBCASE("server/client") { writerVar = server; readerVar = client; }
    SUBCASE("client/server") { writerVar = client; readerVar = server; }
    SUBCASE("client/client") { writerVar = client; readerVar = client; }
    // clang-format on

    CAPTURE(std::holds_alternative<ServerRef>(writerVar));
    CAPTURE(std::holds_alternative<ClientRef>(writerVar));
    CAPTURE(std::holds_alternative<ServerRef>(readerVar));
    CAPTURE(std::holds_alternative<ClientRef>(readerVar));

    std::visit(
        [&](auto writerRef, auto readerRef) {
            auto& writer = writerRef.get();
            auto& reader = readerRef.get();

            const LocalizedText displayName("", "display name");
            CHECK_NOTHROW(services::writeDisplayName(writer, id, displayName));
            CHECK(services::readDisplayName(reader, id) == displayName);

            const LocalizedText description("en-US", "description...");
            CHECK_NOTHROW(services::writeDescription(writer, id, description));
            CHECK(services::readDescription(reader, id) == description);

            const NodeId dataType(0, UA_NS0ID_DOUBLE);
            CHECK_NOTHROW(services::writeDataType(writer, id, dataType));
            CHECK(services::readDataType(reader, id) == dataType);

            const ValueRank valueRank = ValueRank::OneDimension;
            CHECK_NOTHROW(services::writeValueRank(writer, id, valueRank));
            CHECK(services::readValueRank(reader, id) == valueRank);

            std::vector<uint32_t> arrayDimensions{3};
            CHECK_NOTHROW(services::writeArrayDimensions(writer, id, arrayDimensions));
            CHECK(services::readArrayDimensions(reader, id) == arrayDimensions);

            const std::vector<double> array{1, 2, 3};
            const auto variant = Variant::fromArray(array);
            CHECK_NOTHROW(services::writeValue(writer, id, variant));
            Variant variantRead;
            CHECK_NOTHROW(services::readValue(reader, id, variantRead));
            CHECK(variantRead.getArrayCopy<double>() == array);

            const auto dataValue = DataValue::fromArray(array);
            CHECK_NOTHROW(services::writeDataValue(writer, id, dataValue));
            DataValue dataValueRead;
            CHECK_NOTHROW(services::readDataValue(reader, id, dataValueRead));
            CHECK_EQ(dataValueRead->hasValue, true);
            CHECK_EQ(dataValueRead->hasSourceTimestamp, true);
            CHECK_EQ(dataValueRead->hasServerTimestamp, true);
            CHECK(dataValueRead.getValuePtr()->getArrayCopy<double>() == array);
        },
        writerVar,
        readerVar
    );
}

TEST_CASE("Browse") {
    Server server;

    SUBCASE("Browse child") {
        const NodeId rootId{0, UA_NS0ID_ROOTFOLDER};

        CHECK_THROWS(services::browseChild(server, rootId, {}));
        CHECK_THROWS(services::browseChild(server, rootId, {{0, "Invalid"}}));
        CHECK(
            services::browseChild(server, rootId, {{0, "Types"}, {0, "ObjectTypes"}}) ==
            NodeId{0, UA_NS0ID_OBJECTTYPESFOLDER}
        );
    }
}
