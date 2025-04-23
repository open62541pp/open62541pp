#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

#include "open62541pp/client.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/server.hpp"
#include "open62541pp/services/detail/client_service.hpp"
#include "open62541pp/ua/types.hpp"  // ReadResponse

#include "helper/server_runner.hpp"

using Catch::Matchers::Message;
using namespace opcua;

TEST_CASE("AsyncServiceAdapter") {
    SECTION("createCallbackAndContext") {
        using Response = int;
        using Adapter = services::detail::AsyncServiceAdapter<Response>;

        std::optional<Response> result;
        bool throwInCompletionHandler = false;
        detail::ExceptionCatcher catcher;

        auto callbackAndContext = Adapter::createCallbackAndContext(catcher, [&](Response res) {
            result = res;
            if (throwInCompletionHandler) {
                throw std::runtime_error("CompletionHandler");
            }
        });

        const auto invokeCallback = [&](int* response) {
            callbackAndContext.callback(
                nullptr,  // client
                callbackAndContext.context.release(),
                0,  // requestId
                response
            );
        };

        Response response = 5;
        SECTION("Success") {
            invokeCallback(&response);
            CHECK(result.has_value());
            CHECK(result.value() == response);
            CHECK_FALSE(catcher.hasException());
        }
        SECTION("Response nullptr") {
            invokeCallback(nullptr);
            CHECK_FALSE(result.has_value());
            CHECK(catcher.hasException());
            CHECK_THROWS_MATCHES(catcher.rethrow(), BadStatus, Message("BadUnexpectedError"));
        }
        SECTION("Exception in completion handler") {
            throwInCompletionHandler = true;
            invokeCallback(&response);
            CHECK(result.has_value());
            CHECK(result.value() == response);
            CHECK(catcher.hasException());
            CHECK_THROWS_MATCHES(
                catcher.rethrow(), std::runtime_error, Message("CompletionHandler")
            );
        }
    }
}

TEST_CASE("sendRequest") {
    Client client;

    auto sendReadRequest = [&] {
        UA_ReadValueId item{};
        item.nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
        item.attributeId = UA_ATTRIBUTEID_BROWSENAME;
        UA_ReadRequest request{};
        request.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;
        request.nodesToReadSize = 1;
        request.nodesToRead = &item;
        return services::detail::sendRequest<UA_ReadRequest, ReadResponse>(client, request);
    };

    SECTION("Disconnected") {
        const auto response = sendReadRequest();
        // UA_STATUSCODE_BADCONNECTIONCLOSED or UA_STATUSCODE_BADINTERNALERROR (v1.0)
        CHECK(response.responseHeader().serviceResult().isBad());
    }

    Server server;
    ServerRunner serverRunner{server};
    client.connect("opc.tcp://localhost:4840");

    SECTION("Success") {
        const auto response = sendReadRequest();
        CHECK(response.responseHeader().serviceResult().isGood());
        CHECK(response.results()[0].value().scalar<QualifiedName>() == QualifiedName{0, "Objects"});
    }
}

TEST_CASE("sendRequestAsync") {
    Client client;

    auto sendReadRequest = [&](auto&& token) {
        UA_ReadValueId item{};
        item.nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
        item.attributeId = UA_ATTRIBUTEID_BROWSENAME;
        UA_ReadRequest request{};
        request.timestampsToReturn = UA_TIMESTAMPSTORETURN_BOTH;
        request.nodesToReadSize = 1;
        request.nodesToRead = &item;
        return services::detail::sendRequestAsync<UA_ReadRequest, ReadResponse>(
            client, request, std::forward<decltype(token)>(token)
        );
    };

    SECTION("Disconnected") {
        sendReadRequest([&](ReadResponse& response) {
            // UA_STATUSCODE_BADSERVERNOTCONNECTED since v1.1
            CHECK(response.responseHeader().serviceResult().isBad());
        });
    }

    Server server;
    ServerRunner serverRunner{server};
    client.connect("opc.tcp://localhost:4840");

    SECTION("Success") {
        bool executed = false;
        sendReadRequest([&](ReadResponse& response) {
            CHECK(response.responseHeader().serviceResult().isGood());
            CHECK(
                response.results()[0].value().scalar<QualifiedName>() == QualifiedName{0, "Objects"}
            );
            executed = true;
        });
        CHECK_NOTHROW(client.runIterate());
        CHECK(executed);
    }

    SECTION("Exception in user callback") {
        sendReadRequest([](ReadResponse&) { throw std::runtime_error("Error"); });
        CHECK_THROWS_AS(client.runIterate(), std::runtime_error);
    }
}
