#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "open62541pp/config.hpp"
#include "open62541pp/services/attribute.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"
#include "open62541pp/services/nodemanagement.hpp"  // add*
#include "open62541pp/ua/nodeids.hpp"
#include "open62541pp/ua/types.hpp"

#include "helper/server_client_setup.hpp"

using namespace opcua;

TEST_CASE("Attribute service set (highlevel)") {
    Server server;
    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    SECTION("Read initial variable node attributes") {
        VariableAttributes attr;
        attr.setDisplayName({"", "TestAttributes"});
        attr.setDescription({"", "..."});
        attr.setWriteMask(~0U);
        attr.setDataType(DataTypeId::Int32);
        attr.setValueRank(ValueRank::TwoDimensions);
        attr.setArrayDimensions({2, 3});
        attr.setAccessLevel(AccessLevel::CurrentRead | AccessLevel::CurrentWrite);
        attr.setMinimumSamplingInterval(11.11);

        const NodeId id{1, "TestAttributes"};
        REQUIRE(services::addVariable(
            server,
            objectsId,
            id,
            "TestAttributes",
            attr,
            VariableTypeId::BaseDataVariableType,
            ReferenceTypeId::HasComponent
        ));

        CHECK(services::readDisplayName(server, id).value() == attr.displayName());
        CHECK(services::readDescription(server, id).value() == attr.description());
        CHECK(services::readWriteMask(server, id).value() == attr.writeMask());
        CHECK(services::readDataType(server, id).value() == attr.dataType());
        CHECK(services::readValueRank(server, id).value() == attr.valueRank());
        CHECK(
            services::readArrayDimensions(server, id).value() ==
            std::vector(attr.arrayDimensions().begin(), attr.arrayDimensions().end())
        );
        CHECK(services::readAccessLevel(server, id).value() == attr.accessLevel());
        CHECK(
            services::readMinimumSamplingInterval(server, id).value() ==
            attr.minimumSamplingInterval()
        );
    }

    SECTION("Read/write object node attributes") {
        const NodeId id{1, "TestAttributes"};
        REQUIRE(services::addObject(
            server,
            objectsId,
            id,
            "TestAttributes",
            {},
            ObjectTypeId::BaseObjectType,
            ReferenceTypeId::HasComponent
        ));

        // write new attributes
        const auto eventNotifier = EventNotifier::HistoryRead | EventNotifier::HistoryWrite;
        CHECK(services::writeEventNotifier(server, id, eventNotifier).isGood());

        // read new attributes
        CHECK(services::readEventNotifier(server, id).value().allOf(eventNotifier));
    }

    SECTION("Read/write variable node attributes") {
        const NodeId id{1, "TestAttributes"};
        REQUIRE(services::addVariable(
            server,
            objectsId,
            id,
            "TestAttributes",
            {},
            VariableTypeId::BaseDataVariableType,
            ReferenceTypeId::HasComponent
        ));

        // write new attributes
        CHECK(services::writeDisplayName(server, id, {{}, "NewDisplayName"}).isGood());
        CHECK(services::writeDescription(server, id, {{}, "NewDescription"}).isGood());
        CHECK(services::writeWriteMask(server, id, WriteMask::Executable).isGood());
        CHECK(services::writeDataType(server, id, NodeId{0, 2}).isGood());
        CHECK(services::writeValueRank(server, id, ValueRank::TwoDimensions).isGood());
        CHECK(services::writeArrayDimensions(server, id, {3, 2}).isGood());
        const auto newAccessLevel = AccessLevel::CurrentRead | AccessLevel::CurrentWrite;
        CHECK(services::writeAccessLevel(server, id, newAccessLevel).isGood());
        CHECK(services::writeMinimumSamplingInterval(server, id, 10.0).isGood());
        CHECK(services::writeHistorizing(server, id, true).isGood());

        // read new attributes
        // https://github.com/open62541/open62541/issues/6723
        CHECK(services::readDisplayName(server, id).value() == LocalizedText({}, "NewDisplayName"));
        CHECK(services::readDescription(server, id).value() == LocalizedText({}, "NewDescription"));
        CHECK(services::readWriteMask(server, id).value() == UA_WRITEMASK_EXECUTABLE);
        CHECK(services::readDataType(server, id).value() == NodeId(0, 2));
        CHECK(services::readValueRank(server, id).value() == ValueRank::TwoDimensions);
        CHECK(services::readArrayDimensions(server, id).value().size() == 2);
        CHECK(services::readArrayDimensions(server, id).value().at(0) == 3);
        CHECK(services::readArrayDimensions(server, id).value().at(1) == 2);
        CHECK(services::readAccessLevel(server, id).value() == newAccessLevel);
        CHECK(services::readMinimumSamplingInterval(server, id).value() == 10.0);
        CHECK(services::readHistorizing(server, id).value() == true);
    }

#ifdef UA_ENABLE_METHODCALLS
    SECTION("Read/write method node attributes") {
        const NodeId id{1, "TestMethod"};
        REQUIRE(services::addMethod(
            server, objectsId, id, "TestMethod", nullptr, {}, {}, {}, ReferenceTypeId::HasComponent
        ));

        // write new attributes
        CHECK(services::writeExecutable(server, id, true).isGood());

        // read new attributes
        CHECK(services::readExecutable(server, id));
        CHECK(services::readUserExecutable(server, id));
    }
#endif

    SECTION("Read/write reference type node attributes") {
        const NodeId id{1, "TestReferenceType"};
        REQUIRE(services::addReferenceType(
            server,
            {0, UA_NS0ID_ORGANIZES},
            id,
            "TestReferenceType",
            {},
            ReferenceTypeId::HasSubtype
        ));

        // read default attributes
        CHECK(services::readIsAbstract(server, id).value() == false);
        CHECK(services::readSymmetric(server, id).value() == false);
        CHECK(services::readInverseName(server, id).value() == LocalizedText({}, {}));

        // write new attributes
        CHECK(services::writeIsAbstract(server, id, true).isGood());
        CHECK(services::writeSymmetric(server, id, false).isGood());
        CHECK(services::writeInverseName(server, id, LocalizedText({}, "InverseName")).isGood());

        // read new attributes
        CHECK(services::readIsAbstract(server, id).value() == true);
        CHECK(services::readSymmetric(server, id).value() == false);
        CHECK(services::readInverseName(server, id).value() == LocalizedText({}, "InverseName"));
    }

    SECTION("Value rank and array dimension combinations") {
        const NodeId id{1, "TestDimensions"};
        REQUIRE(services::addVariable(
            server,
            objectsId,
            id,
            "TestDimensions",
            {},
            VariableTypeId::BaseDataVariableType,
            ReferenceTypeId::HasComponent
        ));

        SECTION("Unspecified dimension (ValueRank <= 0)") {
            const std::vector<ValueRank> valueRanks = {
                ValueRank::Any,
                ValueRank::Scalar,
                ValueRank::ScalarOrOneDimension,
                ValueRank::OneOrMoreDimensions,
            };
            for (auto valueRank : valueRanks) {
                CAPTURE(valueRank);
                CHECK(services::writeValueRank(server, id, valueRank).isGood());
                CHECK(services::writeArrayDimensions(server, id, {}).isGood());
                CHECK_FALSE(services::writeArrayDimensions(server, id, {1}).isGood());
                CHECK_FALSE(services::writeArrayDimensions(server, id, {1, 2}).isGood());
                CHECK_FALSE(services::writeArrayDimensions(server, id, {1, 2, 3}).isGood());
            }
        }

        SECTION("OneDimension") {
            CHECK(services::writeValueRank(server, id, ValueRank::OneDimension).isGood());
            CHECK(services::writeArrayDimensions(server, id, {1}).isGood());
            CHECK_FALSE(services::writeArrayDimensions(server, id, {}).isGood());
            CHECK_FALSE(services::writeArrayDimensions(server, id, {1, 2}).isGood());
            CHECK_FALSE(services::writeArrayDimensions(server, id, {1, 2, 3}).isGood());
        }

        SECTION("TwoDimensions") {
            CHECK(services::writeValueRank(server, id, ValueRank::TwoDimensions).isGood());
            CHECK(services::writeArrayDimensions(server, id, {1, 2}).isGood());
            CHECK_FALSE(services::writeArrayDimensions(server, id, {}).isGood());
            CHECK_FALSE(services::writeArrayDimensions(server, id, {1}).isGood());
            CHECK_FALSE(services::writeArrayDimensions(server, id, {1, 2, 3}).isGood());
        }

        SECTION("ThreeDimensions") {
            CHECK(services::writeValueRank(server, id, ValueRank::ThreeDimensions).isGood());
            CHECK(services::writeArrayDimensions(server, id, {1, 2, 3}).isGood());
            CHECK_FALSE(services::writeArrayDimensions(server, id, {}).isGood());
            CHECK_FALSE(services::writeArrayDimensions(server, id, {1}).isGood());
            CHECK_FALSE(services::writeArrayDimensions(server, id, {1, 2}).isGood());
        }
    }

    SECTION("Read/write value") {
        const NodeId id{1, "TestValue"};
        REQUIRE(services::addVariable(
            server,
            objectsId,
            id,
            "TestValue",
            {},
            VariableTypeId::BaseDataVariableType,
            ReferenceTypeId::HasComponent
        ));

        Variant variantWrite(11.11);
        services::writeValue(server, id, variantWrite).throwIfBad();

        Variant variantRead = services::readValue(server, id).value();
        CHECK(variantRead.scalar<double>() == 11.11);
    }

    SECTION("Read/write data value") {
        const NodeId id{1, "TestDataValue"};
        REQUIRE(services::addVariable(
            server,
            objectsId,
            id,
            "TestDataValue",
            {},
            VariableTypeId::BaseDataVariableType,
            ReferenceTypeId::HasComponent
        ));

        Variant variant(11);
        DataValue valueWrite(variant, {}, DateTime::now(), {}, uint16_t{1}, UA_STATUSCODE_GOOD);
        services::writeDataValue(server, id, valueWrite).throwIfBad();

        DataValue valueRead = services::readDataValue(server, id).value();

        CHECK(valueRead->hasValue == true);
        CHECK(valueRead->hasServerTimestamp == true);
        CHECK(valueRead->hasSourceTimestamp == true);
        // CHECK(valueRead->hasServerPicoseconds == true);
        // CHECK(valueRead->hasSourcePicoseconds == true);
        CHECK(valueRead->hasStatus == false);  // doesn't contain error code on success

        CHECK(valueRead.value().scalar<int>() == 11);
        CHECK(valueRead->sourceTimestamp == valueWrite->sourceTimestamp);
        CHECK(valueRead->sourcePicoseconds == valueWrite->sourcePicoseconds);
    }

#ifdef UA_ENABLE_TYPEDESCRIPTION

    SECTION("Data type definition (read)") {
        const NodeId id{0, UA_NS0ID_BUILDINFO};
        const Variant variant = services::readDataTypeDefinition(server, id).value();
        CHECK(variant.isScalar());
        CHECK(variant.type() == &UA_TYPES[UA_TYPES_STRUCTUREDEFINITION]);

        const auto definition = variant.scalar<StructureDefinition>();
        CHECK(definition.defaultEncodingId() == NodeId(0, 340));
        CHECK(definition.baseDataType() == NodeId(0, 22));
        CHECK(definition.structureType() == StructureType::Structure);
        CHECK(definition.fields().size() == 6);
    }

    // SECTION("Data type definition (write/read EnumDefinition, not supported yet)") {
    //     const NodeId id{1, "MyEnum"};
    //     services::addDataType(server, {0, UA_NS0ID_ENUMERATION}, id, "MyEnum");

    //     const EnumDefinition definition{{0, "Zero"}, {1, "One"}};
    //     services::writeDataTypeDefinition(server, id, Variant(definition)).value();

    //     const auto definitionRead = services::readDataTypeDefinition(server, id);
    //     CHECK(definitionRead.value().isType<EnumDefinition>());
    // }

#endif
}

TEST_CASE("Attribute service set (highlevel, async)") {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& server = setup.server;
    auto& client = setup.client;
    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    SECTION("Read/write value") {
        // create variable node
        const NodeId id{1, 1000};
        REQUIRE(services::addVariable(
            server,
            objectsId,
            id,
            "Variable",
            VariableAttributes{}.setAccessLevel(
                AccessLevel::CurrentRead | AccessLevel::CurrentWrite
            ),
            VariableTypeId::BaseDataVariableType,
            ReferenceTypeId::HasComponent
        ));

        // write
        {
            auto variant = Variant(11.11);
            auto future = services::writeValueAsync(client, id, variant, useFuture);
            client.runIterate();
            future.get().throwIfBad();
        }

        // read
        {
            auto future = services::readValueAsync(client, id, useFuture);
            client.runIterate();
            CHECK(future.get().value().scalar<double>() == 11.11);
        }
    }

    // sync and async functions use the same attribute handlers, so testing one attribute is enough
}

TEMPLATE_TEST_CASE("Attribute service set write/read", "", Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& client = setup.client;
    auto& connection = setup.instance<TestType>();

    // create variable node
    const NodeId id{1, 1000};
    REQUIRE(services::addVariable(
        setup.server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        id,
        "Variable",
        VariableAttributes{}.setAccessLevel(AccessLevel::CurrentRead | AccessLevel::CurrentWrite),
        VariableTypeId::BaseDataVariableType,
        ReferenceTypeId::HasComponent
    ));

    const double value = 11.11;
    Result<DataValue> result;

    // write
    if constexpr (isAsync<TestType>) {
        auto future = services::writeAttributeAsync(
            connection, id, AttributeId::Value, DataValue(Variant(value)), useFuture
        );
        client.runIterate();
        future.get().throwIfBad();
    } else {
        services::writeAttribute(connection, id, AttributeId::Value, DataValue(Variant(value)))
            .throwIfBad();
    }

    // read
    if constexpr (isAsync<TestType>) {
        auto future = services::readAttributeAsync(
            connection, id, AttributeId::Value, TimestampsToReturn::Neither, useFuture
        );
        client.runIterate();
        result = future.get();
    } else {
        result = services::readAttribute(
            connection, id, AttributeId::Value, TimestampsToReturn::Neither
        );
    }

    CHECK(result.value().value().scalar<double>() == value);
}

TEST_CASE("Attribute service set raw") {
    Client client;

    const std::vector<ReadValueId> nodesToRead{
        {NodeId{1, 1000}, AttributeId::Value},
    };
    const std::vector<WriteValue> nodesToWrite{
        {NodeId{1, 1000}, AttributeId::Value, {}, DataValue{}},
    };

    SECTION("read") {
        const ReadRequest request{{}, {}, TimestampsToReturn{}, nodesToRead};
        CHECK_NOTHROW(services::read(client, request));
        CHECK_NOTHROW(services::readAsync(client, request, useFuture));
    }
    SECTION("read overload") {
        CHECK_NOTHROW(services::read(client, nodesToRead, TimestampsToReturn{}));
        CHECK_NOTHROW(services::readAsync(client, nodesToRead, TimestampsToReturn{}, useFuture));
    }
    SECTION("write") {
        const WriteRequest request{{}, nodesToWrite};
        CHECK_NOTHROW(services::write(client, request));
        CHECK_NOTHROW(services::writeAsync(client, request, useFuture));
    }
    SECTION("write overload") {
        CHECK_NOTHROW(services::write(client, nodesToWrite));
        CHECK_NOTHROW(services::writeAsync(client, nodesToWrite, useFuture));
    }
}
