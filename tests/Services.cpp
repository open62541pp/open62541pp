#include <chrono>
#include <thread>
#include <variant>
#include <vector>

#include <doctest/doctest.h>

#include "open62541pp/Config.h"
#include "open62541pp/Event.h"
#include "open62541pp/NodeIds.h"
#include "open62541pp/services/services.h"
#include "open62541pp/types/DateTime.h"
#include "open62541pp/types/ExtensionObject.h"

#include "helper/ServerClientSetup.h"
#include "helper/discard.h"
#include "helper/stringify.h"

using namespace opcua;
using namespace std::literals::chrono_literals;

TEST_CASE_TEMPLATE("NodeManagement service set", T, Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& server = setup.server;
    auto& connection = setup.getInstance<T>();

    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};
    const NodeId newId{1, 1000};

    SUBCASE("addObject") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addObjectAsync(connection, objectsId, newId, "object");
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addObject(connection, objectsId, newId, "object");
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Object);
    }

    SUBCASE("addFolder") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addFolderAsync(connection, objectsId, newId, "folder");
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addFolder(connection, objectsId, newId, "folder");
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Object);
    }

    SUBCASE("addVariable") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addVariableAsync(connection, objectsId, newId, "variable");
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addVariable(connection, objectsId, newId, "variable");
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Variable);
    }

    SUBCASE("addProperty") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addPropertyAsync(connection, objectsId, newId, "property");
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addProperty(connection, objectsId, newId, "property");
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Variable);
    }

#ifdef UA_ENABLE_METHODCALLS
    SUBCASE("addMethod") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addMethodAsync(
                connection, objectsId, newId, "method", {}, {}, {}
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addMethod(connection, objectsId, newId, "method", {}, {}, {});
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::Method);
    }
#endif

    SUBCASE("addObjectType") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addObjectTypeAsync(
                connection, {0, UA_NS0ID_BASEOBJECTTYPE}, newId, "objecttype"
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addObjectType(
                connection, {0, UA_NS0ID_BASEOBJECTTYPE}, newId, "objecttype"
            );
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::ObjectType);
    }

    SUBCASE("addVariableType") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addVariableTypeAsync(
                connection, {0, UA_NS0ID_BASEVARIABLETYPE}, newId, "variabletype"
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addVariableType(
                connection, {0, UA_NS0ID_BASEVARIABLETYPE}, newId, "variabletype"
            );
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::VariableType);
    }

    SUBCASE("addReferenceType") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addReferenceTypeAsync(
                connection, {0, UA_NS0ID_ORGANIZES}, newId, "referencetype"
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addReferenceType(
                connection, {0, UA_NS0ID_ORGANIZES}, newId, "referencetype"
            );
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::ReferenceType);
    }

    SUBCASE("addDataType") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addDataTypeAsync(
                connection, {0, UA_NS0ID_STRUCTURE}, newId, "datatype"
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addDataType(connection, {0, UA_NS0ID_STRUCTURE}, newId, "datatype");
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::DataType);
    }

    SUBCASE("addView") {
        Result<NodeId> result;
        if constexpr (isAsync<T>) {
            auto future = services::addViewAsync(
                connection, {0, UA_NS0ID_VIEWSFOLDER}, newId, "view"
            );
            setup.client.runIterate();
            result = future.get();
        } else {
            result = services::addView(connection, {0, UA_NS0ID_VIEWSFOLDER}, newId, "view");
        }
        CHECK_EQ(result.value(), newId);
        CHECK_EQ(services::readNodeClass(server, newId).value(), NodeClass::View);
    }

    SUBCASE("Add/delete reference") {
        services::addFolder(connection, objectsId, {1, 1000}, "folder").value();
        services::addObject(connection, objectsId, {1, 1001}, "object").value();

        auto addReference = [&] {
            if constexpr (isAsync<T>) {
                auto future = services::addReferenceAsync(
                    connection, {1, 1000}, {1, 1001}, ReferenceTypeId::Organizes
                );
                setup.client.runIterate();
                return future.get();
            } else {
                return services::addReference(
                    connection, {1, 1000}, {1, 1001}, ReferenceTypeId::Organizes
                );
            }
        };
        CHECK(addReference().code().isGood());
        CHECK(addReference().code() == UA_STATUSCODE_BADDUPLICATEREFERENCENOTALLOWED);

        auto deleteReference = [&] {
            if constexpr (isAsync<T>) {
                auto future = services::deleteReferenceAsync(
                    connection, {1, 1000}, {1, 1001}, ReferenceTypeId::Organizes, true, true
                );
                setup.client.runIterate();
                return future.get();
            } else {
                return services::deleteReference(
                    connection, {1, 1000}, {1, 1001}, ReferenceTypeId::Organizes, true, true
                );
            }
        };
        CHECK(deleteReference().code().isGood());
    }

    SUBCASE("Delete node") {
        services::addObject(connection, objectsId, {1, 1000}, "object").value();

        auto deleteNode = [&] {
            if constexpr (isAsync<T>) {
                auto future = services::deleteNodeAsync(connection, {1, 1000});
                setup.client.runIterate();
                return future.get();
            } else {
                return services::deleteNode(connection, {1, 1000});
            }
        };
        CHECK(deleteNode().code().isGood());
        CHECK(deleteNode().code() == UA_STATUSCODE_BADNODEIDUNKNOWN);
    }

    SUBCASE("Random node id") {
        // https://www.open62541.org/doc/1.3/server.html#node-addition-and-deletion
        const auto id = services::addObject(connection, objectsId, {1, 0}, "random").value();
        CHECK(id != NodeId(1, 0));
        CHECK(id.getNamespaceIndex() == 1);
    }
}

TEST_CASE("Attribute service set (highlevel)") {
    Server server;
    const NodeId objectsId{0, UA_NS0ID_OBJECTSFOLDER};

    SUBCASE("Read default variable node attributes") {
        const NodeId id{1, "testAttributes"};
        services::addVariable(server, objectsId, id, "testAttributes").value();

        CHECK(services::readNodeId(server, id).value() == id);
        CHECK(services::readNodeClass(server, id).value() == NodeClass::Variable);
        CHECK(services::readBrowseName(server, id).value() == QualifiedName(1, "testAttributes"));
        // CHECK(services::readDisplayName(server, id), LocalizedText("", "testAttributes"));
        CHECK(services::readDescription(server, id).value().getText().empty());
        CHECK(services::readDescription(server, id).value().getLocale().empty());
        CHECK(services::readWriteMask(server, id).value() == 0);
        const uint32_t adminUserWriteMask = 0xFFFFFFFF;  // all bits set
        CHECK(services::readUserWriteMask(server, id).value() == adminUserWriteMask);
        CHECK(services::readDataType(server, id).value() == NodeId(0, UA_NS0ID_BASEDATATYPE));
        CHECK(services::readValueRank(server, id).value() == ValueRank::Any);
        CHECK(services::readArrayDimensions(server, id).value().empty());
        CHECK(services::readAccessLevel(server, id).value() == AccessLevel::CurrentRead);
        const uint8_t adminUserAccessLevel = 0xFF;  // all bits set
        CHECK(services::readUserAccessLevel(server, id).value() == adminUserAccessLevel);
        CHECK(services::readMinimumSamplingInterval(server, id).value() == 0.0);
        CHECK(services::readHistorizing(server, id).value() == false);
    }

    SUBCASE("Read initial variable node attributes") {
        VariableAttributes attr;
        attr.setDisplayName({"", "testAttributes"});
        attr.setDescription({"", "..."});
        attr.setWriteMask(~0U);
        attr.setDataType(DataTypeId::Int32);
        attr.setValueRank(ValueRank::TwoDimensions);
        attr.setArrayDimensions({2, 3});
        attr.setAccessLevel(AccessLevel::CurrentRead | AccessLevel::CurrentWrite);
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
        )
            .value();

        CHECK(services::readDisplayName(server, id).value() == attr.getDisplayName());
        CHECK(services::readDescription(server, id).value() == attr.getDescription());
        CHECK(services::readWriteMask(server, id).value() == attr.getWriteMask());
        CHECK(services::readDataType(server, id).value() == attr.getDataType());
        CHECK(services::readValueRank(server, id).value() == attr.getValueRank());
        CHECK(
            services::readArrayDimensions(server, id).value() ==
            std::vector<uint32_t>(attr.getArrayDimensions())
        );
        CHECK(services::readAccessLevel(server, id).value() == attr.getAccessLevel());
        CHECK(
            services::readMinimumSamplingInterval(server, id).value() ==
            attr.getMinimumSamplingInterval()
        );
    }

    SUBCASE("Read/write object node attributes") {
        const NodeId id{1, "testAttributes"};
        services::addObject(server, objectsId, id, "testAttributes").value();

        // write new attributes
        const auto eventNotifier = EventNotifier::HistoryRead | EventNotifier::HistoryWrite;
        CHECK(services::writeEventNotifier(server, id, eventNotifier).code().isGood());

        // read new attributes
        CHECK(services::readEventNotifier(server, id).value().allOf(eventNotifier));
    }

    SUBCASE("Read/write variable node attributes") {
        const NodeId id{1, "testAttributes"};
        services::addVariable(server, objectsId, id, "testAttributes").value();

        // write new attributes
        CHECK(services::writeDisplayName(server, id, {"en-US", "newDisplayName"}).code().isGood());
        CHECK(services::writeDescription(server, id, {"de-DE", "newDescription"}).code().isGood());
        CHECK(services::writeWriteMask(server, id, WriteMask::Executable).code().isGood());
        CHECK(services::writeDataType(server, id, NodeId{0, 2}).code().isGood());
        CHECK(services::writeValueRank(server, id, ValueRank::TwoDimensions).code().isGood());
        CHECK(services::writeArrayDimensions(server, id, {3, 2}).code().isGood());
        const auto newAccessLevel = AccessLevel::CurrentRead | AccessLevel::CurrentWrite;
        CHECK(services::writeAccessLevel(server, id, newAccessLevel).code().isGood());
        CHECK(services::writeMinimumSamplingInterval(server, id, 10.0).code().isGood());
        CHECK(services::writeHistorizing(server, id, true).code().isGood());

        // read new attributes
        CHECK(
            services::readDisplayName(server, id).value() ==
            LocalizedText("en-US", "newDisplayName")
        );
        CHECK(
            services::readDescription(server, id).value() ==
            LocalizedText("de-DE", "newDescription")
        );
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
    SUBCASE("Read/write method node attributes") {
        const NodeId id{1, "testMethod"};
        services::addMethod(server, objectsId, id, "testMethod", nullptr, {}, {}).value();

        // write new attributes
        CHECK(services::writeExecutable(server, id, true).code().isGood());

        // read new attributes
        CHECK(services::readExecutable(server, id));
        CHECK(services::readUserExecutable(server, id));
    }
#endif

    SUBCASE("Read/write reference node attributes") {
        const NodeId id{0, UA_NS0ID_REFERENCES};

        // read default attributes
        CHECK(services::readIsAbstract(server, id).value() == true);
        CHECK(services::readSymmetric(server, id).value() == true);
        CHECK(services::readInverseName(server, id).value() == LocalizedText("", "References"));

        // write new attributes
        CHECK(services::writeIsAbstract(server, id, false).code().isGood());
        CHECK(services::writeSymmetric(server, id, false).code().isGood());
        CHECK(services::writeInverseName(server, id, LocalizedText("", "New")).code().isGood());

        // read new attributes
        CHECK(services::readIsAbstract(server, id).value() == false);
        CHECK(services::readSymmetric(server, id).value() == false);
        CHECK(services::readInverseName(server, id).value() == LocalizedText("", "New"));
    }

    SUBCASE("Value rank and array dimension combinations") {
        const NodeId id{1, "testDimensions"};
        services::addVariable(server, objectsId, id, "testDimensions").value();

        SUBCASE("Unspecified dimension (ValueRank <= 0)") {
            const std::vector<ValueRank> valueRanks = {
                ValueRank::Any,
                ValueRank::Scalar,
                ValueRank::ScalarOrOneDimension,
                ValueRank::OneOrMoreDimensions,
            };
            for (auto valueRank : valueRanks) {
                CAPTURE(valueRank);
                CHECK(services::writeValueRank(server, id, valueRank).code().isGood());
                CHECK(services::writeArrayDimensions(server, id, {}).code().isGood());
                CHECK(services::writeArrayDimensions(server, id, {1}).code().isBad());
                CHECK(services::writeArrayDimensions(server, id, {1, 2}).code().isBad());
                CHECK(services::writeArrayDimensions(server, id, {1, 2, 3}).code().isBad());
            }
        }

        SUBCASE("OneDimension") {
            CHECK(services::writeValueRank(server, id, ValueRank::OneDimension).code().isGood());
            CHECK(services::writeArrayDimensions(server, id, {1}).code().isGood());
            CHECK(services::writeArrayDimensions(server, id, {}).code().isBad());
            CHECK(services::writeArrayDimensions(server, id, {1, 2}).code().isBad());
            CHECK(services::writeArrayDimensions(server, id, {1, 2, 3}).code().isBad());
        }

        SUBCASE("TwoDimensions") {
            CHECK(services::writeValueRank(server, id, ValueRank::TwoDimensions).code().isGood());
            CHECK(services::writeArrayDimensions(server, id, {1, 2}).code().isGood());
            CHECK(services::writeArrayDimensions(server, id, {}).code().isBad());
            CHECK(services::writeArrayDimensions(server, id, {1}).code().isBad());
            CHECK(services::writeArrayDimensions(server, id, {1, 2, 3}).code().isBad());
        }

        SUBCASE("ThreeDimensions") {
            CHECK(services::writeValueRank(server, id, ValueRank::ThreeDimensions).code().isGood());
            CHECK(services::writeArrayDimensions(server, id, {1, 2, 3}).code().isGood());
            CHECK(services::writeArrayDimensions(server, id, {}).code().isBad());
            CHECK(services::writeArrayDimensions(server, id, {1}).code().isBad());
            CHECK(services::writeArrayDimensions(server, id, {1, 2}).code().isBad());
        }
    }

    SUBCASE("Read/write value") {
        const NodeId id{1, "testValue"};
        services::addVariable(server, objectsId, id, "testValue").value();

        Variant variantWrite;
        variantWrite.setScalarCopy(11.11);
        CHECK(services::writeValue(server, id, variantWrite).code().isGood());

        Variant variantRead = services::readValue(server, id).value();
        CHECK(variantRead.getScalar<double>() == 11.11);
    }

    SUBCASE("Read/write data value") {
        const NodeId id{1, "testDataValue"};
        services::addVariable(server, objectsId, id, "testDataValue").value();

        Variant variant;
        variant.setScalarCopy<int>(11);
        DataValue valueWrite(variant, {}, DateTime::now(), {}, uint16_t{1}, UA_STATUSCODE_GOOD);
        CHECK(services::writeDataValue(server, id, valueWrite).code().isGood());

        DataValue valueRead = services::readDataValue(server, id).value();

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

TEST_CASE_TEMPLATE("Attribute service set write/read", T, Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& client = setup.client;
    auto& connection = setup.getInstance<T>();

    // create variable node
    const NodeId id{1, 1000};
    services::addVariable(
        setup.server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        id,
        "variable",
        VariableAttributes{}.setAccessLevel(AccessLevel::CurrentRead | AccessLevel::CurrentWrite)
    )
        .value();

    const double value = 11.11;
    Result<DataValue> result;

    // write
    if constexpr (isAsync<T>) {
        auto future = services::writeAttributeAsync(
            connection, id, AttributeId::Value, DataValue::fromScalar(value)
        );
        client.runIterate();
        future.get().code().throwIfBad();
    } else {
        services::writeAttribute(connection, id, AttributeId::Value, DataValue::fromScalar(value))
            .code()
            .throwIfBad();
    }

    // read
    if constexpr (isAsync<T>) {
        auto future = services::readAttributeAsync(connection, id, AttributeId::Value);
        client.runIterate();
        result = future.get();
    } else {
        result = services::readAttribute(connection, id, AttributeId::Value);
    }

    CHECK(result.value().getValue().getScalar<double>() == value);
}

TEST_CASE_TEMPLATE("View service set", T, Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& server = setup.server;
    auto& client = setup.client;
    auto& connection = setup.getInstance<T>();

    // add node to query references
    const NodeId id{1, 1000};
    services::addVariable(server, {0, UA_NS0ID_OBJECTSFOLDER}, id, "Variable").value();

    SUBCASE("browse") {
        const BrowseDescription bd(id, BrowseDirection::Both);
        Result<BrowseResult> result;

        if constexpr (isAsync<T>) {
            auto future = services::browseAsync(connection, bd);
            client.runIterate();
            result = future.get();
        } else {
            result = services::browse(connection, bd);
        }

        CHECK(result.value().getStatusCode().isGood());
        CHECK(result.value().getContinuationPoint().empty());

        const auto refs = result.value().getReferences();
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
        Result<BrowseResult> result;

        // restrict browse result to max 1 reference, more with browseNext
        result = services::browse(connection, bd, 1);
        CHECK(result.value().getStatusCode().isGood());
        CHECK(result.value().getContinuationPoint().empty() == false);
        CHECK(result.value().getReferences().size() == 1);

        auto browseNext = [&](bool releaseContinuationPoint) {
            if constexpr (isAsync<T>) {
                auto future = services::browseNextAsync(
                    connection, releaseContinuationPoint, result.value().getContinuationPoint()
                );
                client.runIterate();
                result = future.get();
            } else {
                result = services::browseNext(
                    connection, releaseContinuationPoint, result.value().getContinuationPoint()
                );
            }
        };

        // get next result
        browseNext(false);
        CHECK(result.value().getStatusCode().isGood());
        CHECK(result.value().getContinuationPoint().empty() == false);
        CHECK(result.value().getReferences().size() == 1);

        // release continuation point, result should be empty
        browseNext(true);
        CHECK(result.value().getStatusCode().isGood());
        CHECK(result.value().getContinuationPoint().empty());
        CHECK(result.value().getReferences().size() == 0);
    }

    if constexpr (isServer<T>) {
        SUBCASE("browseRecursive") {
            const BrowseDescription bd(
                ObjectId::Server,
                BrowseDirection::Forward,
                ReferenceTypeId::References,
                true,
                UA_NODECLASS_VARIABLE
            );

            const auto ids = services::browseRecursive(connection, bd).value();
            CHECK(!ids.empty());

            auto contains = [&](const NodeId& element) {
                return std::find(ids.begin(), ids.end(), ExpandedNodeId(element)) != ids.end();
            };

            CHECK(contains(VariableId::Server_ServerStatus));
            CHECK(contains(VariableId::Server_ServerStatus_BuildInfo));
            CHECK(contains(VariableId::Server_ServerStatus_BuildInfo_SoftwareVersion));
        }
    }

    SUBCASE("browseAll") {
        const BrowseDescription bd(id, BrowseDirection::Both);
        CHECK(services::browseAll(connection, bd, 0).value().size() == 2);
        CHECK(services::browseAll(connection, bd, 1).value().size() == 1);
    }

    SUBCASE("browseSimplifiedBrowsePath") {
        Result<BrowsePathResult> result;
        if constexpr (isAsync<T>) {
            auto future = services::browseSimplifiedBrowsePathAsync(
                connection, {0, UA_NS0ID_ROOTFOLDER}, {{0, "Objects"}, {1, "Variable"}}
            );
            client.runIterate();
            result = future.get();
        } else {
            result = services::browseSimplifiedBrowsePath(
                connection, {0, UA_NS0ID_ROOTFOLDER}, {{0, "Objects"}, {1, "Variable"}}
            );
        }
        CHECK(result.value().getStatusCode().isGood());
        const auto targets = result.value().getTargets();
        CHECK(targets.size() == 1);
        // https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8
        // value shall be equal to the maximum value of uint32 if all elements processed
        CHECK(targets[0].getRemainingPathIndex() == 0xffffffff);
        CHECK(targets[0].getTargetId().getNodeId() == id);
    }

    SUBCASE("Register/unregister nodes") {
        {
            RegisterNodesResponse response;
            RegisterNodesRequest request({}, {{1, 1000}});
            if constexpr (isAsync<T>) {
                auto future = services::registerNodesAsync(client, request);
                client.runIterate();
                response = future.get();
            } else {
                response = services::registerNodes(client, request);
            }
            CHECK(response.getRegisteredNodeIds().size() == 1);
            CHECK(response.getRegisteredNodeIds()[0] == NodeId(1, 1000));
        }
        {
            UnregisterNodesResponse response;
            UnregisterNodesRequest request({}, {{1, 1000}});
            if constexpr (isAsync<T>) {
                auto future = services::unregisterNodesAsync(client, request);
                client.runIterate();
                response = future.get();
            } else {
                response = services::unregisterNodes(client, request);
            }
            CHECK(response.getResponseHeader().getServiceResult().isGood());
        }
    }
}

#ifdef UA_ENABLE_METHODCALLS
TEST_CASE_TEMPLATE("Method service set", T, Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& connection = setup.getInstance<T>();

    const NodeId objectsId{ObjectId::ObjectsFolder};
    const NodeId methodId{1, 1000};

    bool throwException = false;
    services::addMethod(
        setup.server,
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
    )
        .value();

    auto call = [&](auto&&... args) {
        if constexpr (isAsync<T>) {
            auto future = services::callAsync(std::forward<decltype(args)>(args)...);
            setup.client.runIterate();
            return future.get();
        } else {
            return services::call(std::forward<decltype(args)>(args)...);
        }
    };

    if constexpr (isClient<T>) {
        SUBCASE("Check result (raw)") {
            const CallRequest request(
                {},
                {CallMethodRequest(
                    objectsId,
                    methodId,
                    Span<const Variant>{
                        Variant::fromScalar(int32_t{1}),
                        Variant::fromScalar(int32_t{2}),
                    }
                )}
            );
            const CallResponse response = call(connection, request);
            CHECK(response.getResults().size() == 1);
            CHECK(response.getResults()[0].getStatusCode().isGood());
            CHECK(response.getResults()[0].getOutputArguments().size() == 1);
            CHECK(response.getResults()[0].getOutputArguments()[0].getScalarCopy<int32_t>() == 3);
        }
    }

    SUBCASE("Check result") {
        Result<std::vector<Variant>> result = call(
            connection,
            objectsId,
            methodId,
            Span<const Variant>{
                Variant::fromScalar(int32_t{1}),
                Variant::fromScalar(int32_t{2}),
            }
        );
        CHECK(result.value().size() == 1);
        CHECK(result.value()[0].getScalarCopy<int32_t>() == 3);
    }

    SUBCASE("Propagate exception") {
        throwException = true;
        auto result = call(
            connection,
            objectsId,
            methodId,
            Span<const Variant>{
                Variant::fromScalar(int32_t{1}),
                Variant::fromScalar(int32_t{2}),
            }
        );
        CHECK(result.code() == UA_STATUSCODE_BADUNEXPECTEDERROR);
    }

    SUBCASE("Invalid input arguments") {
        auto result = call(
            connection,
            objectsId,
            methodId,
            Span<const Variant>{
                Variant::fromScalar(true),
                Variant::fromScalar(11.11f),
            }
        );
        CHECK(result.code() == UA_STATUSCODE_BADINVALIDARGUMENT);
    }

    SUBCASE("Missing arguments") {
        auto result = call(connection, objectsId, methodId, Span<const Variant>{});
        CHECK(result.code() == UA_STATUSCODE_BADARGUMENTSMISSING);
    }

    SUBCASE("Too many arguments") {
        auto result = call(
            connection,
            objectsId,
            methodId,
            Span<const Variant>{
                Variant::fromScalar(int32_t{1}),
                Variant::fromScalar(int32_t{2}),
                Variant::fromScalar(int32_t{3}),
            }
        );
        CHECK(result.code() == UA_STATUSCODE_BADTOOMANYARGUMENTS);
    }
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
        const auto subId = services::createSubscription(client, parameters).value();
        CAPTURE(subId);
    }

    SUBCASE("modifySubscription") {
        const auto subId = services::createSubscription(client, parameters).value();

        parameters.priority = 1;
        CHECK(services::modifySubscription(client, subId, parameters).code().isGood());
        CHECK(
            services::modifySubscription(client, subId + 1, parameters).code() ==
            UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID
        );
    }

    SUBCASE("setPublishingMode") {
        const auto subId = services::createSubscription(client, parameters).value();
        CHECK(services::setPublishingMode(client, subId, false).code().isGood());
    }

    SUBCASE("deleteSubscription") {
        const auto subId = services::createSubscription(client, parameters).value();
        CHECK(services::deleteSubscription(client, subId).code().isGood());
        CHECK(
            services::deleteSubscription(client, subId + 1).code() ==
            UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID
        );
    }

    SUBCASE("deleteSubscription with callback") {
        bool deleted = false;
        const auto subId = services::createSubscription(client, parameters, true, [&](uint32_t) {
                               deleted = true;
                           }).value();

        CHECK(services::deleteSubscription(client, subId).code().isGood());
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
    services::addVariable(server, {0, UA_NS0ID_OBJECTSFOLDER}, id, "Variable").value();

    services::SubscriptionParameters subscriptionParameters{};
    services::MonitoringParametersEx monitoringParameters{};
    monitoringParameters.samplingInterval = 0.0;  // fastest

    SUBCASE("createMonitoredItemDataChange without subscription") {
        CHECK_FALSE(services::createMonitoredItemDataChange(
            client,
            11U,  // random subId
            {id, AttributeId::Value},
            MonitoringMode::Reporting,
            monitoringParameters,
            {}
        ));
    }

    const auto subId = services::createSubscription(client, subscriptionParameters).value();
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

        CHECK(services::writeValue(server, id, Variant::fromScalar(11.11)).code().isGood());
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
        const auto monId =
            services::createMonitoredItemDataChange(
                client,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {}
            )
                .value();
        CAPTURE(monId);

        services::MonitoringParametersEx modifiedParameters{};
        modifiedParameters.samplingInterval = 1000.0;
        CHECK(
            services::modifyMonitoredItem(client, subId, monId, modifiedParameters).code().isGood()
        );
        CHECK(modifiedParameters.samplingInterval == 1000.0);  // should not be revised
    }

    SUBCASE("setMonitoringMode") {
        const auto monId =
            services::createMonitoredItemDataChange(
                client,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {}
            )
                .value();
        CAPTURE(monId);
        CHECK(services::setMonitoringMode(client, subId, monId, MonitoringMode::Disabled)
                  .code()
                  .isGood());
    }

#if UAPP_OPEN62541_VER_GE(1, 2)
    SUBCASE("setTriggering") {
        // use current server time as triggering item and let it trigger the variable node
        size_t notificationCountTriggering = 0;
        size_t notificationCount = 0;
        const auto monIdTriggering =
            services::createMonitoredItemDataChange(
                client,
                subId,
                {VariableId::Server_ServerStatus_CurrentTime, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                [&](uint32_t, uint32_t, const DataValue&) { notificationCountTriggering++; }
            ).value();
        // set triggered item's monitoring mode to sampling
        // -> will only report if triggered by triggering item
        // https://reference.opcfoundation.org/Core/Part4/v105/docs/5.12.1.6
        const auto monId =
            services::createMonitoredItemDataChange(
                client,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Sampling,
                monitoringParameters,
                [&](uint32_t, uint32_t, const DataValue&) { notificationCount++; }
            ).value();

        client.runIterate();
        CHECK(notificationCountTriggering > 0);
        CHECK(notificationCount == 0);  // no triggering links yet

        auto result = services::setTriggering(
            client,
            subId,
            monIdTriggering,
            {monId},  // links to add
            {}  // links to remove
        );
        CHECK(result.code().isGood());

        client.runIterate();
        CHECK(notificationCount > 0);
    }
#endif

    SUBCASE("deleteMonitoredItem") {
        CHECK(
            services::deleteMonitoredItem(client, subId, 11U).code() ==
            UA_STATUSCODE_BADMONITOREDITEMIDINVALID
        );

        bool deleted = false;
        const auto monId =
            services::createMonitoredItemDataChange(
                client,
                subId,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {},
                [&](uint32_t, uint32_t) { deleted = true; }
            ).value();

        CHECK(services::deleteMonitoredItem(client, subId, monId).code().isGood());
        client.runIterate();
        CHECK(deleted == true);
    }
}

TEST_CASE("MonitoredItem service set (server)") {
    Server server;
    const NodeId id{1, 1000};
    services::addVariable(server, {0, UA_NS0ID_OBJECTSFOLDER}, id, "Variable").value();

    services::MonitoringParametersEx monitoringParameters{};
    monitoringParameters.samplingInterval = 0.0;  // fastest

    SUBCASE("createMonitoredItemDataChange") {
        size_t notificationCount = 0;
        const auto monId =
            services::createMonitoredItemDataChange(
                server,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                [&](uint32_t, uint32_t, const DataValue&) { notificationCount++; }
            ).value();
        CAPTURE(monId);
        std::this_thread::sleep_for(100ms);
        services::writeValue(server, id, Variant::fromScalar(11.11)).code().throwIfBad();
        server.runIterate();
        CHECK(notificationCount > 0);
    }

    SUBCASE("deleteMonitoredItem") {
        CHECK(
            services::deleteMonitoredItem(server, 11U).code() ==
            UA_STATUSCODE_BADMONITOREDITEMIDINVALID
        );

        const auto monId =
            services::createMonitoredItemDataChange(
                server,
                {id, AttributeId::Value},
                MonitoringMode::Reporting,
                monitoringParameters,
                {}
            )
                .value();
        CAPTURE(monId);
        CHECK(services::deleteMonitoredItem(server, monId).code().isGood());
    }
}
#endif
