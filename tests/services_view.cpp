#include <doctest/doctest.h>

#include "open62541pp/nodeids.hpp"
#include "open62541pp/services/nodemanagement.hpp"
#include "open62541pp/services/view.hpp"

#include "helper/server_client_setup.hpp"

using namespace opcua;

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
        BrowseResult result;

        if constexpr (isAsync<T>) {
            auto future = services::browseAsync(connection, bd);
            client.runIterate();
            result = future.get();
        } else {
            result = services::browse(connection, bd);
        }

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
        BrowseResult result;

        // restrict browse result to max 1 reference, more with browseNext
        result = services::browse(connection, bd, 1);
        CHECK(result.getStatusCode().isGood());
        CHECK(result.getContinuationPoint().empty() == false);
        CHECK(result.getReferences().size() == 1);

        auto browseNext = [&](bool releaseContinuationPoint) {
            if constexpr (isAsync<T>) {
                auto future = services::browseNextAsync(
                    connection, releaseContinuationPoint, result.getContinuationPoint()
                );
                client.runIterate();
                result = future.get();
            } else {
                result = services::browseNext(
                    connection, releaseContinuationPoint, result.getContinuationPoint()
                );
            }
        };

        // get next result
        browseNext(false);
        CHECK(result.getStatusCode().isGood());
        CHECK(result.getContinuationPoint().empty() == false);
        CHECK(result.getReferences().size() == 1);

        // release continuation point, result should be empty
        browseNext(true);
        CHECK(result.getStatusCode().isGood());
        CHECK(result.getContinuationPoint().empty());
        CHECK(result.getReferences().size() == 0);
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
        BrowsePathResult result;
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
        CHECK(result.getStatusCode().isGood());
        CHECK(result.getTargets().size() == 1);
        // https://reference.opcfoundation.org/Core/Part4/v105/docs/5.8
        // value shall be equal to the maximum value of uint32 if all elements processed
        CHECK(result.getTargets()[0].getRemainingPathIndex() == 0xffffffff);
        CHECK(result.getTargets()[0].getTargetId().getNodeId() == id);
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
