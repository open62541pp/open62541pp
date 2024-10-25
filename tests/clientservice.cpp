#include <future>

#include <doctest/doctest.h>

#include "open62541pp/client.hpp"
#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/server.hpp"
#include "open62541pp/services/detail/client_services.hpp"
#include "open62541pp/services/detail/response_handling.hpp"
#include "open62541pp/types_composed.hpp"  // ReadResponse

#include "helper/server_runner.hpp"

using namespace opcua;

TEST_CASE("AsyncServiceAdapter") {
    SUBCASE("createCallbackAndContext") {
        using Response = int;
        using Adapter = services::detail::AsyncServiceAdapter<Response>;

        std::optional<Response> result;
        bool throwInTransform = false;
        bool throwInCompletionHandler = false;
        detail::ExceptionCatcher catcher;

        auto callbackAndContext = Adapter::createCallbackAndContext(
            &catcher,
            [&](int val) {
                if (throwInTransform) {
                    throw std::runtime_error("Transform");
                }
                return val;
            },
            [&](Response res) {
                result = res;
                if (throwInCompletionHandler) {
                    throw std::runtime_error("CompletionHandler");
                }
            }
        );

        auto invokeCallback = [&](int* response) {
            callbackAndContext.callback(
                nullptr,  // client
                callbackAndContext.context.release(),
                0,  // requestId
                response
            );
        };

        Response response = 5;
        SUBCASE("Success") {
            invokeCallback(&response);
            CHECK(result.has_value());
            CHECK(result.value() == response);
            CHECK_FALSE(catcher.hasException());
        }
        SUBCASE("Response nullptr") {
            invokeCallback(nullptr);
            CHECK_FALSE(result.has_value());
            CHECK(catcher.hasException());
            CHECK_THROWS_AS_MESSAGE(catcher.rethrow(), BadStatus, "BadUnexpectedError");
        }
        SUBCASE("Exception in transform function") {
            throwInTransform = true;
            invokeCallback(&response);
            CHECK_FALSE(result.has_value());
            CHECK(catcher.hasException());
            CHECK_THROWS_AS_MESSAGE(catcher.rethrow(), std::runtime_error, "Transform");
        }
        SUBCASE("Exception in completion handler") {
            throwInCompletionHandler = true;
            invokeCallback(&response);
            CHECK(result.has_value());
            CHECK(result.value() == response);
            CHECK(catcher.hasException());
            CHECK_THROWS_AS_MESSAGE(catcher.rethrow(), std::runtime_error, "CompletionHandler");
        }
    }
}

TEST_CASE("sendRequest") {
    Client client;

    auto sendReadRequest = [&](auto&& transform, auto&& token) {
        UA_ReadValueId item{};
        item.nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
        item.attributeId = UA_ATTRIBUTEID_BROWSENAME;
        UA_ReadRequest request{};
        request.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;
        request.nodesToReadSize = 1;
        request.nodesToRead = &item;

        return services::detail::sendRequest<UA_ReadRequest, UA_ReadResponse>(
            client,
            request,
            std::forward<decltype(transform)>(transform),
            std::forward<decltype(token)>(token)
        );
    };

    SUBCASE("Async disconnected") {
        sendReadRequest(services::detail::Wrap<ReadResponse>{}, [&](ReadResponse& response) {
            // UA_STATUSCODE_BADSERVERNOTCONNECTED since v1.1
            CHECK(response.getResponseHeader().getServiceResult().isBad());
        });
    }

    SUBCASE("Sync disconnected") {
        const auto response = sendReadRequest(
            services::detail::Wrap<ReadResponse>{}, services::detail::SyncOperation{}
        );
        // UA_STATUSCODE_BADCONNECTIONCLOSED or UA_STATUSCODE_BADINTERNALERROR (v1.0)
        CHECK(response.getResponseHeader().getServiceResult().isBad());
    }

    Server server;
    ServerRunner serverRunner(server);
    client.connect("opc.tcp://localhost:4840");

    auto checkReadResponse = [](const ReadResponse& response) {
        CHECK(response.getResponseHeader().getServiceResult().isGood());
        CHECK_EQ(
            response.getResults()[0].getValue().getScalar<QualifiedName>(),
            QualifiedName(0, "Objects")
        );
    };

    SUBCASE("Async") {
        SUBCASE("Success") {
            sendReadRequest(services::detail::Wrap<ReadResponse>{}, [&](ReadResponse& response) {
                checkReadResponse(response);
            });
            CHECK_NOTHROW(client.runIterate());
        }

        SUBCASE("Exception in transform function") {
            sendReadRequest(
                [](UA_ReadResponse&) -> bool { throw std::runtime_error("Error"); }, [](bool) {}
            );
            CHECK_THROWS_AS_MESSAGE(client.runIterate(), std::runtime_error, "Error");
        }

        SUBCASE("Exception in user callback") {
            sendReadRequest(services::detail::Wrap<ReadResponse>{}, [](ReadResponse&) {
                throw std::runtime_error("Error");
            });
            CHECK_THROWS_AS_MESSAGE(client.runIterate(), std::runtime_error, "Error");
        }
    }

    SUBCASE("Sync") {
        SUBCASE("Success") {
            const auto response = sendReadRequest(
                services::detail::Wrap<ReadResponse>{}, services::detail::SyncOperation{}
            );
            checkReadResponse(response);
        }

        SUBCASE("Exception in transform function") {
            CHECK_THROWS_AS_MESSAGE(
                sendReadRequest(
                    [](UA_ReadResponse&) -> int { throw std::runtime_error("Error"); },
                    services::detail::SyncOperation{}
                ),
                std::runtime_error,
                "Error"
            );
        }
    }
}
