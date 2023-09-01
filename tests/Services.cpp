#include <chrono>
#include <thread>
#include <utility>  // pair
#include <variant>
#include <vector>

#include <doctest/doctest.h>

#include "open62541pp/Client.h"
#include "open62541pp/Config.h"
#include "open62541pp/Event.h"
#include "open62541pp/NodeIds.h"
#include "open62541pp/Server.h"
#include "open62541pp/services/services.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/ExtensionObject.h"

#include "helper/Runner.h"

using namespace opcua;
using namespace std::literals::chrono_literals;

TEST_CASE("NodeManagement service set (server & client)") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;
    client.connect("opc.tcp://localhost:4840");
    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    const auto testNodeManagement = [&](auto& serverOrClient) {
        SUBCASE("Non-type nodes") {
            services::addObject(serverOrClient, objectsId, {1, 1000}, "object");
            CHECK(services::readNodeClass(server, {1, 1000}) == NodeClass::Object);

            services::addFolder(serverOrClient, objectsId, {1, 1001}, "folder");
            CHECK(services::readNodeClass(server, {1, 1001}) == NodeClass::Object);

            services::addVariable(serverOrClient, objectsId, {1, 1002}, "variable");
            CHECK(services::readNodeClass(server, {1, 1002}) == NodeClass::Variable);

            services::addProperty(serverOrClient, objectsId, {1, 1003}, "property");
            CHECK(services::readNodeClass(server, {1, 1003}) == NodeClass::Variable);

#ifdef UA_ENABLE_METHODCALLS
            services::addMethod(serverOrClient, objectsId, {1, 1004}, "method", {}, {}, {});
            CHECK(services::readNodeClass(server, {1, 1004}) == NodeClass::Method);
#endif
        }

        SUBCASE("Type nodes") {
            services::addObjectType(
                serverOrClient, {0, UA_NS0ID_BASEOBJECTTYPE}, {1, 1000}, "objecttype"
            );
            CHECK(services::readNodeClass(server, {1, 1000}) == NodeClass::ObjectType);

            services::addVariableType(
                serverOrClient, {0, UA_NS0ID_BASEVARIABLETYPE}, {1, 1001}, "variabletype"
            );
            CHECK(services::readNodeClass(server, {1, 1001}) == NodeClass::VariableType);

            services::addReferenceType(
                serverOrClient, {0, UA_NS0ID_ORGANIZES}, {1, 1002}, "referenceType"
            );
            CHECK(services::readNodeClass(server, {1, 1002}) == NodeClass::ReferenceType);

            services::addDataType(serverOrClient, {0, UA_NS0ID_STRUCTURE}, {1, 1003}, "dataType");
            CHECK(services::readNodeClass(server, {1, 1003}) == NodeClass::DataType);
        }

        SUBCASE("View nodes") {
            services::addView(serverOrClient, {0, UA_NS0ID_VIEWSFOLDER}, {1, 1000}, "view");
            CHECK(services::readNodeClass(server, {1, 1000}) == NodeClass::View);
        }

        SUBCASE("Add/delete reference") {
            services::addFolder(serverOrClient, objectsId, {1, 1000}, "folder");
            services::addObject(serverOrClient, objectsId, {1, 1001}, "object");
            services::addReference(
                serverOrClient, {1, 1000}, {1, 1001}, ReferenceTypeId::Organizes
            );
            CHECK_THROWS_WITH(
                services::addReference(
                    serverOrClient, {1, 1000}, {1, 1001}, ReferenceTypeId::Organizes
                ),
                "BadDuplicateReferenceNotAllowed"
            );
            CHECK_NOTHROW(services::deleteReference(
                serverOrClient, {1, 1000}, {1, 1001}, ReferenceTypeId::Organizes, true, true
            ));
        }

        SUBCASE("Delete node") {
            services::addObject(serverOrClient, objectsId, {1, 1000}, "object");
            services::deleteNode(serverOrClient, {1, 1000});
            CHECK_THROWS_WITH(services::deleteNode(serverOrClient, {1, 1000}), "BadNodeIdUnknown");
        }
    };

    // clang-format off
    SUBCASE("Server") { testNodeManagement(server); };
    SUBCASE("Client") { testNodeManagement(client); };
    // clang-format on
}

TEST_CASE("Attribute service set (server)") {
    Server server;
    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    SUBCASE("Read default attributes") {
        const NodeId id{1, "testAttributes"};
        services::addVariable(server, objectsId, id, "testAttributes");

        CHECK(services::readNodeId(server, id) == id);
        CHECK(services::readNodeClass(server, id) == NodeClass::Variable);
        CHECK(services::readBrowseName(server, id) == QualifiedName(1, "testAttributes"));
        //_EQ CHECK(services::readDisplayName(server, id), LocalizedText("", "testAttributes"));
        CHECK(services::readDescription(server, id).getText().empty());
        CHECK(services::readDescription(server, id).getLocale().empty());
        CHECK(services::readWriteMask(server, id) == 0);
        const uint32_t adminUserWriteMask = 0xFFFFFFFF;  // all bits set
        CHECK(services::readUserWriteMask(server, id) == adminUserWriteMask);
        CHECK(services::readDataType(server, id) == NodeId(0, UA_NS0ID_BASEDATATYPE));
        CHECK(services::readValueRank(server, id) == ValueRank::Any);
        CHECK(services::readArrayDimensions(server, id).empty());
        CHECK(services::readAccessLevel(server, id) == UA_ACCESSLEVELMASK_READ);
        const uint8_t adminUserAccessLevel = 0xFF;  // all bits set
        CHECK(services::readUserAccessLevel(server, id) == adminUserAccessLevel);
        CHECK(services::readMinimumSamplingInterval(server, id) == 0.0);
    }

    SUBCASE("Read initial attributes") {
        VariableAttributes attr;
        attr.setDisplayName({"", "testAttributes"});
        attr.setDescription({"", "..."});
        attr.setWriteMask(~0U);
        attr.setDataType(DataTypeId::Int32);
        attr.setValueRank(ValueRank::TwoDimensions);
        attr.setArrayDimensions({2, 3});
        attr.setAccessLevel(UA_ACCESSLEVELMASK_READ);
        attr.setMinimumSamplingInterval(11.11);

        const NodeId id{1, "testAttributes"};
        services::addVariable(
            server,
            objectsId,
            id,
            "testAttributes",
            attr,
            VariableTypeId::BaseDataVariableType,
            ReferenceTypeId::HasComponent
        );

        CHECK(services::readDisplayName(server, id) == attr.getDisplayName());
        CHECK(services::readDescription(server, id) == attr.getDescription());
        CHECK(services::readWriteMask(server, id) == attr.getWriteMask());
        CHECK(services::readDataType(server, id) == attr.getDataType());
        CHECK(services::readValueRank(server, id) == attr.getValueRank());
        CHECK(
            services::readArrayDimensions(server, id) ==
            std::vector<uint32_t>(attr.getArrayDimensions())
        );
        CHECK(services::readAccessLevel(server, id) == attr.getAccessLevel());
        CHECK(
            services::readMinimumSamplingInterval(server, id) == attr.getMinimumSamplingInterval()
        );
    }

    SUBCASE("Read/write node attributes") {
        const NodeId id{1, "testAttributes"};
        services::addVariable(server, objectsId, id, "testAttributes");

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

        Variant variantRead = services::readValue(server, id);
        CHECK(variantRead.getScalar<double>() == 11.11);
    }

    SUBCASE("Read/write data value") {
        const NodeId id{1, "testDataValue"};
        services::addVariable(server, objectsId, id, "testDataValue");

        Variant variant;
        variant.setScalarCopy<int>(11);
        DataValue valueWrite(variant, {}, DateTime::now(), {}, uint16_t{1}, UA_STATUSCODE_GOOD);
        services::writeDataValue(server, id, valueWrite);

        DataValue valueRead = services::readDataValue(server, id);

        CHECK_EQ(valueRead->hasValue, true);
        CHECK_EQ(valueRead->hasServerTimestamp, true);
        CHECK_EQ(valueRead->hasSourceTimestamp, true);
        CHECK_EQ(valueRead->hasServerPicoseconds, true);
        CHECK_EQ(valueRead->hasSourcePicoseconds, true);
        CHECK_EQ(valueRead->hasStatus, false);  // doesn't contain error code on success

        CHECK(valueRead.getValue().getScalar<int>() == 11);
        CHECK(valueRead->sourceTimestamp == valueWrite->sourceTimestamp);
        CHECK(valueRead->sourcePicoseconds == valueWrite->sourcePicoseconds);
    }
}

TEST_CASE("Attribute service set (server & client)") {
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

    // check remaining attributes with possible writer/reader combinations
    const auto testWriteReadAttribute = [&](auto& writer, auto& reader) {
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
        CHECK(services::readValue(reader, id).template getArrayCopy<double>() == array);

        const auto dataValue = DataValue::fromArray(array);
        CHECK_NOTHROW(services::writeDataValue(writer, id, dataValue));
        DataValue dataValueRead = services::readDataValue(reader, id);
        CHECK_EQ(dataValueRead->hasValue, true);
        CHECK_EQ(dataValueRead->hasSourceTimestamp, true);
        CHECK_EQ(dataValueRead->hasServerTimestamp, true);
        CHECK(dataValueRead.getValue().getArrayCopy<double>() == array);
    };

    // clang-format off
    SUBCASE("Server/Server") { testWriteReadAttribute(server, server); }
    SUBCASE("Server/Client") { testWriteReadAttribute(server, client); }
    SUBCASE("Client/Server") { testWriteReadAttribute(client, server); }
    SUBCASE("Client/Client") { testWriteReadAttribute(client, client); }
    // clang-format on
}

TEST_CASE("View service set (server & client)") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;
    client.connect("opc.tcp://localhost:4840");

    // add node to query references
    const NodeId id{1, 1000};
    services::addVariable(server, {0, UA_NS0ID_OBJECTSFOLDER}, id, "Variable");

    const auto testBrowse = [&](auto& serverOrClient) {
        SUBCASE("browse") {
            const BrowseDescription bd(id, BrowseDirection::Both);
            const auto result = services::browse(serverOrClient, bd);

            CHECK(result.getStatusCode().isGood());
            CHECK(result.getContinuationPoint().empty());

            const auto refs = result.getReferences();
            CHECK(refs.size() == 2);
            // 1. ComponentOf Objects
            CHECK(refs[0].getReferenceTypeId() == NodeId(0, UA_NS0ID_HASCOMPONENT));
            CHECK(refs[0].getIsForward() == false);
            CHECK(refs[0].getNodeId() == ExpandedNodeId({0, UA_NS0ID_OBJECTSFOLDER}));
            CHECK(refs[0].getBrowseName() == QualifiedName(0, "Objects"));
            // 2. HasTypeDefinition BaseDataVariableType
            CHECK(refs[1].getReferenceTypeId() == NodeId(0, UA_NS0ID_HASTYPEDEFINITION));
            CHECK(refs[1].getIsForward() == true);
            CHECK(refs[1].getNodeId() == ExpandedNodeId({0, UA_NS0ID_BASEDATAVARIABLETYPE}));
            CHECK(refs[1].getBrowseName() == QualifiedName(0, "BaseDataVariableType"));
        }

        SUBCASE("browseNext") {
            // https://github.com/open62541/open62541/blob/v1.3.5/tests/client/check_client_highlevel.c#L252-L318
            const BrowseDescription bd({0, UA_NS0ID_SERVER}, BrowseDirection::Both);
            // restrict browse result to max 1 reference, more with browseNext
            auto resultBrowse = services::browse(serverOrClient, bd, 1);

            CHECK(resultBrowse.getStatusCode().isGood());
            CHECK(resultBrowse.getContinuationPoint().empty() == false);
            CHECK(resultBrowse.getReferences().size() == 1);

            // get next result
            resultBrowse = services::browseNext(
                serverOrClient, false, resultBrowse.getContinuationPoint()
            );
            CHECK(resultBrowse.getStatusCode().isGood());
            CHECK(resultBrowse.getContinuationPoint().empty() == false);
            CHECK(resultBrowse.getReferences().size() == 1);

            // release continuation point, result should be empty
            resultBrowse = services::browseNext(
                serverOrClient, true, resultBrowse.getContinuationPoint()
            );
            CHECK(resultBrowse.getStatusCode().isGood());
            CHECK(resultBrowse.getContinuationPoint().empty());
            CHECK(resultBrowse.getReferences().size() == 0);
        }

        SUBCASE("browseAll") {
            const BrowseDescription bd(id, BrowseDirection::Both);
            CHECK(services::browseAll(serverOrClient, bd, 0).size() == 2);
            CHECK(services::browseAll(serverOrClient, bd, 1).size() == 1);
        }

        SUBCASE("browseSimplifiedBrowsePath") {
            const auto result = services::browseSimplifiedBrowsePath(
                serverOrClient, {0, UA_NS0ID_ROOTFOLDER}, {{0, "Objects"}, {1, "Variable"}}
            );
            CHECK(result.getStatusCode().isGood());
            const auto targets = result.getTargets();
            CHECK(targets.size() == 1);
            // https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8
            // value shall be equal to the maximum value of uint32 if all elements processed
            CHECK(targets[0].getRemainingPathIndex() == 0xffffffff);
            CHECK(targets[0].getTargetId().getNodeId() == id);
        }
    };

    // clang-format off
    SUBCASE("Server") { testBrowse(server); };
    SUBCASE("Client") { testBrowse(client); };
    // clang-format on
}

TEST_CASE("View service set (server)") {
    Server server;

    SUBCASE("browseRecursive") {
        const BrowseDescription bd(
            ObjectId::Server,
            BrowseDirection::Forward,
            ReferenceTypeId::References,
            true,
            UA_NODECLASS_VARIABLE
        );

        const auto results = services::browseRecursive(server, bd);
        CHECK(!results.empty());

        auto contains = [&](const NodeId& id) {
            return std::find(results.begin(), results.end(), ExpandedNodeId(id)) != results.end();
        };

        CHECK(contains(VariableId::Server_ServerStatus));
        CHECK(contains(VariableId::Server_ServerStatus_BuildInfo));
        CHECK(contains(VariableId::Server_ServerStatus_BuildInfo_SoftwareVersion));
    }
}

#ifdef UA_ENABLE_METHODCALLS
TEST_CASE("Method service set (server & client)") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;
    client.connect("opc.tcp://localhost:4840");

    const NodeId objectsId{ObjectId::ObjectsFolder};
    const NodeId methodId{1, 1000};

    bool throwException = false;
    services::addMethod(
        server,
        objectsId,
        methodId,
        "add",
        [&](Span<const Variant> inputs, Span<Variant> outputs) {
            if (throwException) {
                throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
            }
            const auto a = inputs[0].getScalarCopy<int32_t>();
            const auto b = inputs[1].getScalarCopy<int32_t>();
            outputs[0].setScalarCopy(a + b);
        },
        {
            Argument("a", {"en-US", "first number"}, DataTypeId::Int32, ValueRank::Scalar),
            Argument("b", {"en-US", "second number"}, DataTypeId::Int32, ValueRank::Scalar),
        },
        {
            Argument("sum", {"en-US", "sum of both numbers"}, DataTypeId::Int32, ValueRank::Scalar),
        }
    );

    const auto testCall = [&](auto& serverOrClient) {
        SUBCASE("Check result") {
            const std::vector<Variant> outputs = services::call(
                serverOrClient,
                objectsId,
                methodId,
                {
                    Variant::fromScalar<int32_t>(1),
                    Variant::fromScalar<int32_t>(2),
                }
            );
            CHECK(outputs.size() == 1);
            CHECK(outputs.at(0).getScalarCopy<int32_t>() == 3);
        }

        SUBCASE("Propagate exception") {
            throwException = true;
            CHECK_THROWS_WITH(
                services::call(
                    serverOrClient,
                    objectsId,
                    methodId,
                    {
                        Variant::fromScalar<int32_t>(1),
                        Variant::fromScalar<int32_t>(2),
                    }
                ),
                "BadUnexpectedError"
            );
        }

        SUBCASE("Invalid input arguments") {
            CHECK_THROWS_WITH(
                services::call(
                    serverOrClient,
                    objectsId,
                    methodId,
                    {
                        Variant::fromScalar<bool>(true),
                        Variant::fromScalar<float>(11.11f),
                    }
                ),
                "BadInvalidArgument"
            );
            CHECK_THROWS_WITH(
                services::call(serverOrClient, objectsId, methodId, {}), "BadArgumentsMissing"
            );
            CHECK_THROWS_WITH(
                services::call(
                    serverOrClient,
                    objectsId,
                    methodId,
                    {
                        Variant::fromScalar<int32_t>(1),
                        Variant::fromScalar<int32_t>(2),
                        Variant::fromScalar<int32_t>(3),
                    }
                ),
                "BadTooManyArguments"
            );
        }
    };

    // clang-format off
    SUBCASE("Server") { testCall(server); };
    SUBCASE("Client") { testCall(client); };
    // clang-format on
}
#endif

#ifdef UA_ENABLE_SUBSCRIPTIONS
TEST_CASE("Subscription service set (client)") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;
    client.connect("opc.tcp://localhost:4840");

    services::SubscriptionParameters parameters{};

    SUBCASE("createSubscription") {
        const auto subId = services::createSubscription(client, parameters);
        CAPTURE(subId);
    }

    SUBCASE("modifySubscription") {
        const auto subId = services::createSubscription(client, parameters);

        parameters.priority = 1;
        CHECK_NOTHROW(services::modifySubscription(client, subId, parameters));
        CHECK_THROWS_WITH(
            services::modifySubscription(client, subId + 1, parameters), "BadSubscriptionIdInvalid"
        );
    }

    SUBCASE("setPublishingMode") {
        const auto subId = services::createSubscription(client, parameters);

        CHECK_NOTHROW(services::setPublishingMode(client, subId, false));
    }

    SUBCASE("deleteSubscription") {
        const auto subId = services::createSubscription(client, parameters);

        CHECK_NOTHROW(services::deleteSubscription(client, subId));
        CHECK_THROWS_WITH(
            services::deleteSubscription(client, subId + 1), "BadSubscriptionIdInvalid"
        );
    }

    SUBCASE("deleteSubscription with callback") {
        bool deleted = false;
        const auto subId = services::createSubscription(client, parameters, true, [&](uint32_t) {
            deleted = true;
        });

        CHECK_NOTHROW(services::deleteSubscription(client, subId));
        CHECK(deleted == true);
    }
}

TEST_CASE("MonitoredItem service set (client)") {
    Server server;
    ServerRunner serverRunner(server);
    Client client;
    client.connect("opc.tcp://localhost:4840");

    // add variable node to test data change notifications
    const NodeId id{1, 1000};
    services::addVariable(server, {0, UA_NS0ID_OBJECTSFOLDER}, id, "Variable");

    services::SubscriptionParameters subscriptionParameters{};
    services::MonitoringParameters monitoringParameters{};

    SUBCASE("createMonitoredItemDataChange without subscription") {
        CHECK_THROWS(
            services::createMonitoredItemDataChange(
                client,
                11U,  // random subId
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {}
            ) == 1
        );
    }

    const auto subId = services::createSubscription(client, subscriptionParameters);
    CAPTURE(subId);

    SUBCASE("createMonitoredItemDataChange") {
        size_t notificationCount = 0;
        DataValue changedValue;
        const auto monId = services::createMonitoredItemDataChange(
            client,
            subId,
            {id, AttributeId::Value},
            MonitoringMode::Reporting,
            monitoringParameters,
            [&](uint32_t, uint32_t, const DataValue& value) {
                notificationCount++;
                changedValue = value;
            }
        );
        CAPTURE(monId);

        services::writeValue(server, id, Variant::fromScalar(11.11));
        client.runIterate();
        CHECK(notificationCount > 0);
        CHECK(changedValue.getValue().getScalar<double>() == 11.11);
    }

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
    SUBCASE("createMonitoredItemEvent") {
        const EventFilter eventFilter(
            // select clause
            {
                {ObjectTypeId::BaseEventType, {{0, "Time"}}, AttributeId::Value},
                {ObjectTypeId::BaseEventType, {{0, "Severity"}}, AttributeId::Value},
                {ObjectTypeId::BaseEventType, {{0, "Message"}}, AttributeId::Value},
            },
            // where clause
            {}
        );
        monitoringParameters.filter = ExtensionObject::fromDecodedCopy(eventFilter);

        size_t notificationCount = 0;
        size_t eventFieldsSize = 0;
        const auto monId = services::createMonitoredItemEvent(
            client,
            subId,
            {ObjectId::Server, AttributeId::EventNotifier},
            MonitoringMode::Reporting,
            monitoringParameters,
            [&](uint32_t, uint32_t, Span<const Variant> eventFields) {
                notificationCount++;
                eventFieldsSize = eventFields.size();
            }
        );
        CAPTURE(monId);

        Event event(server);
        event.writeTime(DateTime::now());
        event.trigger();
        client.runIterate();
        CHECK(notificationCount == 1);
        CHECK(eventFieldsSize == 3);
    }
#endif

    SUBCASE("modifyMonitoredItem") {
        const auto monId = services::createMonitoredItemDataChange(
            client,
            subId,
            {id, AttributeId::Value},
            MonitoringMode::Reporting,
            monitoringParameters,
            {}
        );
        CAPTURE(monId);

        services::MonitoringParameters modifiedParameters{};
        modifiedParameters.samplingInterval = 1000.0;
        CHECK_NOTHROW(services::modifyMonitoredItem(client, subId, monId, modifiedParameters));
        CHECK(modifiedParameters.samplingInterval == 1000.0);  // should not be revised
    }

    SUBCASE("setMonitoringMode") {
        const auto monId = services::createMonitoredItemDataChange(
            client,
            subId,
            {id, AttributeId::Value},
            MonitoringMode::Reporting,
            monitoringParameters,
            {}
        );
        CAPTURE(monId);
        CHECK_NOTHROW(services::setMonitoringMode(client, subId, monId, MonitoringMode::Disabled));
    }

#if UAPP_OPEN62541_VER_GE(1, 2)
    SUBCASE("setTriggering") {
        // use current server time as triggering item and let it trigger the variable node
        size_t notificationCountTriggering = 0;
        size_t notificationCount = 0;
        const auto monIdTriggering = services::createMonitoredItemDataChange(
            client,
            subId,
            {VariableId::Server_ServerStatus_CurrentTime, AttributeId::Value},
            MonitoringMode::Reporting,
            monitoringParameters,
            [&](uint32_t, uint32_t, const DataValue&) { notificationCountTriggering++; }
        );
        // set triggered item's monitoring mode to sampling
        // -> will only report if triggered by triggering item
        // https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.1.6
        const auto monId = services::createMonitoredItemDataChange(
            client,
            subId,
            {id, AttributeId::Value},
            MonitoringMode::Sampling,
            monitoringParameters,
            [&](uint32_t, uint32_t, const DataValue&) { notificationCount++; }
        );

        client.runIterate();
        CHECK(notificationCountTriggering > 0);
        CHECK(notificationCount == 0);  // no triggering links yet

        services::setTriggering(
            client,
            subId,
            monIdTriggering,
            {monId},  // links to add
            {}  // links to remove
        );

        client.runIterate();
        CHECK(notificationCount > 0);
    }
#endif

    SUBCASE("deleteMonitoredItem") {
        CHECK_THROWS_WITH(
            services::deleteMonitoredItem(client, subId, 11U), "BadMonitoredItemIdInvalid"
        );

        bool deleted = false;
        const auto monId = services::createMonitoredItemDataChange(
            client,
            subId,
            {id, AttributeId::Value},
            MonitoringMode::Reporting,
            monitoringParameters,
            {},
            [&](uint32_t, uint32_t) { deleted = true; }
        );

        CHECK_NOTHROW(services::deleteMonitoredItem(client, subId, monId));
        client.runIterate();
        CHECK(deleted == true);
    }
}

TEST_CASE("MonitoredItem service set (server)") {
    Server server;

    services::MonitoringParameters monitoringParameters{};

    SUBCASE("createMonitoredItemDataChange") {
        size_t notificationCount = 0;
        const auto monId = services::createMonitoredItemDataChange(
            server,
            {VariableId::Server_ServerStatus_CurrentTime, AttributeId::Value},
            MonitoringMode::Reporting,
            monitoringParameters,
            [&](uint32_t, uint32_t, const DataValue&) { notificationCount++; }
        );
        CAPTURE(monId);
        std::this_thread::sleep_for(100ms);
        server.runIterate();
        CHECK(notificationCount > 0);
    }

    SUBCASE("deleteMonitoredItem") {
        CHECK_THROWS_WITH(services::deleteMonitoredItem(server, 11U), "BadMonitoredItemIdInvalid");

        const auto monId = services::createMonitoredItemDataChange(
            server,
            {VariableId::Server_ServerStatus_CurrentTime, AttributeId::Value},
            MonitoringMode::Reporting,
            monitoringParameters,
            {}
        );

        CHECK_NOTHROW(services::deleteMonitoredItem(server, monId));
    }
}
#endif
