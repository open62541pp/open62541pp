#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "open62541pp/services/nodemanagement.hpp"
#include "open62541pp/services/view.hpp"
#include "open62541pp/ua/nodeids.hpp"

#include "helper/server_client_setup.hpp"

using namespace opcua;

TEMPLATE_TEST_CASE("View service set", "", Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& server = setup.server;
    auto& client = setup.client;
    auto& connection = setup.instance<TestType>();

    // add node to query references
    const NodeId id{1, 1000};
    REQUIRE(services::addVariable(
        server,
        {0, UA_NS0ID_OBJECTSFOLDER},
        id,
        "Variable",
        {},
        VariableTypeId::BaseDataVariableType,
        ReferenceTypeId::HasComponent
    ));

    SECTION("browse") {
        const BrowseDescription bd{id, BrowseDirection::Both};
        BrowseResult result;

        if constexpr (isAsync<TestType>) {
            auto future = services::browseAsync(connection, bd, 0, useFuture);
            client.runIterate();
            result = future.get();
        } else {
            result = services::browse(connection, bd, 0);
        }

        CHECK(result.statusCode().isGood());
        CHECK(result.continuationPoint().empty());

        const auto refs = result.references();
        CHECK(refs.size() == 2);
        // 1. ComponentOf Objects
        CHECK(refs[0].referenceTypeId() == NodeId{0, UA_NS0ID_HASCOMPONENT});
        CHECK(refs[0].isForward() == false);
        CHECK(refs[0].nodeId() == ExpandedNodeId{{0, UA_NS0ID_OBJECTSFOLDER}});
        CHECK(refs[0].browseName() == QualifiedName{0, "Objects"});
        // 2. HasTypeDefinition BaseDataVariableType
        CHECK(refs[1].referenceTypeId() == NodeId{0, UA_NS0ID_HASTYPEDEFINITION});
        CHECK(refs[1].isForward() == true);
        CHECK(refs[1].nodeId() == ExpandedNodeId{{0, UA_NS0ID_BASEDATAVARIABLETYPE}});
        CHECK(refs[1].browseName() == QualifiedName{0, "BaseDataVariableType"});
    }

    SECTION("browseNext") {
        // https://github.com/open62541/open62541/blob/v1.3.5/tests/client/check_client_highlevel.c#L252-L318
        const BrowseDescription bd{{0, UA_NS0ID_SERVER}, BrowseDirection::Both};
        BrowseResult result;

        // restrict browse result to max 1 reference, more with browseNext
        result = services::browse(connection, bd, 1);
        CHECK(result.statusCode().isGood());
        CHECK(result.continuationPoint().empty() == false);
        CHECK(result.references().size() == 1);

        auto browseNext = [&](bool releaseContinuationPoint) {
            if constexpr (isAsync<TestType>) {
                auto future = services::browseNextAsync(
                    connection, releaseContinuationPoint, result.continuationPoint(), useFuture
                );
                client.runIterate();
                result = future.get();
            } else {
                result = services::browseNext(
                    connection, releaseContinuationPoint, result.continuationPoint()
                );
            }
        };

        // get next result
        browseNext(false);
        CHECK(result.statusCode().isGood());
        CHECK(result.continuationPoint().empty() == false);
        CHECK(result.references().size() == 1);

        // release continuation point, result should be empty
        browseNext(true);
        CHECK(result.statusCode().isGood());
        CHECK(result.continuationPoint().empty());
        CHECK(result.references().size() == 0);
    }

    SECTION("browseAll") {
        const BrowseDescription bd{id, BrowseDirection::Both};
        CHECK(services::browseAll(connection, bd).value().size() == 2);
    }

    SECTION("browseSimplifiedBrowsePath") {
        BrowsePathResult result;
        if constexpr (isAsync<TestType>) {
            auto future = services::browseSimplifiedBrowsePathAsync(
                connection, {0, UA_NS0ID_ROOTFOLDER}, {{0, "Objects"}, {1, "Variable"}}, useFuture
            );
            client.runIterate();
            result = future.get();
        } else {
            result = services::browseSimplifiedBrowsePath(
                connection, {0, UA_NS0ID_ROOTFOLDER}, {{0, "Objects"}, {1, "Variable"}}
            );
        }
        CHECK(result.statusCode().isGood());
        CHECK(result.targets().size() == 1);
        // https://reference.opcfoundation.org/Core/Part4/v105/docs/5.9
        // value shall be equal to the maximum value of uint32 if all elements processed
        CHECK(result.targets()[0].remainingPathIndex() == 0xffffffff);
        CHECK(result.targets()[0].targetId().nodeId() == id);
    }

    SECTION("Register/unregister nodes") {
        {
            RegisterNodesResponse response;
            RegisterNodesRequest request{{}, {{1, 1000}}};
            if constexpr (isAsync<TestType>) {
                auto future = services::registerNodesAsync(client, request, useFuture);
                client.runIterate();
                response = future.get();
            } else {
                response = services::registerNodes(client, request);
            }
            CHECK(response.registeredNodeIds().size() == 1);
            CHECK(response.registeredNodeIds()[0] == NodeId{1, 1000});
        }
        {
            UnregisterNodesResponse response;
            UnregisterNodesRequest request{{}, {{1, 1000}}};
            if constexpr (isAsync<TestType>) {
                auto future = services::unregisterNodesAsync(client, request, useFuture);
                client.runIterate();
                response = future.get();
            } else {
                response = services::unregisterNodes(client, request);
            }
            CHECK(response.responseHeader().serviceResult().isGood());
        }
    }
}
