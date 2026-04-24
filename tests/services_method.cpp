#include <functional>  // hash
#include <thread>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include "open62541pp/config.hpp"
#include "open62541pp/services/method.hpp"
#include "open62541pp/services/nodemanagement.hpp"  // addMethod, setMethodCallback

#include "helper/macros.hpp"
#include "helper/server_client_setup.hpp"

using namespace opcua;

#ifdef UA_ENABLE_METHODCALLS
TEMPLATE_TEST_CASE("Method service set", "", Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& connection = setup.instance<TestType>();

    const NodeId objectId{ObjectId::ObjectsFolder};
    const NodeId methodId{1, 1000};

    bool throwException = false;
    REQUIRE(
        services::addMethod(
            setup.server,
            objectId,
            methodId,
            "Add",
            [&](Span<const Variant> inputs, Span<Variant> outputs) {
                if (throwException) {
                    throw BadStatus{UA_STATUSCODE_BADUNEXPECTEDERROR};
                }
                const auto a = inputs.at(0).scalar<int32_t>();
                const auto b = inputs.at(1).scalar<int32_t>();
                outputs.at(0) = a + b;
            },
            {
                Argument{"a", {"en-US", "first number"}, DataTypeId::Int32, ValueRank::Scalar},
                Argument{"b", {"en-US", "second number"}, DataTypeId::Int32, ValueRank::Scalar},
            },
            {
                Argument{
                    "sum", {"en-US", "sum of both numbers"}, DataTypeId::Int32, ValueRank::Scalar
                },
            },
            MethodAttributes{},
            ReferenceTypeId::HasComponent
        )
    );

    auto call = [&](auto&&... args) {
        if constexpr (isAsync<TestType>) {
            auto future = services::callAsync(args..., useFuture);
            setup.client.runIterate();
            return future.get();
        } else {
            return services::call(args...);
        }
    };

    SECTION("Check result") {
        const CallMethodResult result = call(
            connection,
            objectId,
            methodId,
            Span<const Variant>{
                Variant{int32_t{1}},
                Variant{int32_t{2}},
            }
        );
        CHECK(result.statusCode().isGood());
        CHECK(result.outputArguments().size() == 1);
        CHECK(result.outputArguments().at(0).scalar<int32_t>() == 3);
    }

    SECTION("Propagate exception") {
        throwException = true;
        const CallMethodResult result = call(
            connection,
            objectId,
            methodId,
            Span<const Variant>{
                Variant{int32_t{1}},
                Variant{int32_t{2}},
            }
        );
        CHECK(result.statusCode() == UA_STATUSCODE_BADUNEXPECTEDERROR);
    }

    SECTION("Invalid input arguments") {
        const CallMethodResult result = call(
            connection,
            objectId,
            methodId,
            Span<const Variant>{
                Variant{true},
                Variant(11.11f),
            }
        );
        CHECK(result.statusCode() == UA_STATUSCODE_BADINVALIDARGUMENT);
    }

    SECTION("Missing arguments") {
        const CallMethodResult result = call(connection, objectId, methodId, Span<const Variant>{});
        CHECK(result.statusCode() == UA_STATUSCODE_BADARGUMENTSMISSING);
    }

    SECTION("Too many arguments") {
        const CallMethodResult result = call(
            connection,
            objectId,
            methodId,
            Span<const Variant>{
                Variant{int32_t{1}},
                Variant{int32_t{2}},
                Variant{int32_t{3}},
            }
        );
        CHECK(result.statusCode() == UA_STATUSCODE_BADTOOMANYARGUMENTS);
    }
}

TEST_CASE("Method service set (full callback signature)") {
    Server server;
    ServerRunner runner{server};

    const NodeId objectId{ObjectId::ObjectsFolder};
    const NodeId methodId{1, 1000};

    bool executed = false;
    NodeId callbackSessionId;
    NodeId callbackMethodId;
    NodeId callbackObjectId;

    REQUIRE(
        services::addMethod(
            server,
            objectId,
            methodId,
            "Method",
            [&](Session& session,
                Span<const Variant>,
                Span<Variant>,
                const NodeId& methodId_,
                const NodeId& objectId_) {
                executed = true;
                callbackSessionId = session.id();
                callbackMethodId = methodId_;
                callbackObjectId = objectId_;
                return UA_STATUSCODE_GOOD;
            },
            {},
            {},
            MethodAttributes{},
            ReferenceTypeId::HasComponent
        )
    );

    const auto result = services::call(server, objectId, methodId, {});
    CHECK(result.statusCode().isGood());

    CHECK(executed);
    CHECK_FALSE(callbackSessionId.isNull());
    CHECK(callbackMethodId == methodId);
    CHECK(callbackObjectId == objectId);
}

TEST_CASE("Method set callback") {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);

    const NodeId objectId{ObjectId::ObjectsFolder};
    const NodeId methodId{1, 1000};

    // create method node without callback via client
    REQUIRE(
        services::addMethod(
            setup.client,
            objectId,
            methodId,
            "TestMethod",
            []([[maybe_unused]] Span<const Variant> inputs,
               [[maybe_unused]] Span<Variant> outputs) {},
            {},
            {},
            MethodAttributes{},
            ReferenceTypeId::HasComponent
        )
    );

    bool executed = false;

    // add method callback via server
    REQUIRE(
        services::setMethodCallback(
            setup.server,
            methodId,
            [&]([[maybe_unused]] Span<const Variant> inputs,
                [[maybe_unused]] Span<Variant> outputs) { executed = true; }
        ).isGood()
    );

    const auto result = services::call(setup.server, objectId, methodId, {});
    CHECK(result.statusCode().isGood());
    CHECK(executed);
}

#if UAPP_HAS_ASYNC_OPERATIONS
static size_t getThreadId() {
    return std::hash<std::thread::id>{}(std::this_thread::get_id());
}

TEST_CASE("Method calls with async operations") {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);

    const NodeId objectId{ObjectId::ObjectsFolder};
    const NodeId methodId{1, 1000};

    REQUIRE(
        services::addMethod(
            setup.server,
            objectId,
            methodId,
            "GetWorkerThreadId",
            []([[maybe_unused]] Span<const Variant> inputs, Span<Variant> outputs) {
                outputs.at(0) = static_cast<uint64_t>(getThreadId());
            },
            {},
            {
                Argument("id", {"en-US", "Thread id"}, DataTypeId::UInt64, ValueRank::Scalar),
            },
            MethodAttributes{},
            ReferenceTypeId::HasComponent
        )
    );

    SECTION("Sync operation") {
        auto future = services::callAsync(setup.client, objectId, methodId, {}, useFuture);
        setup.client.runIterate();
        const auto result = future.get();
        CHECK(result.statusCode().isGood());
        CHECK(result.outputArguments().at(0).to<uint64_t>() != getThreadId());
    }

    // TODO: hangs with thread sanitizer enabled
#ifndef UAPP_TSAN_ENABLED
    SECTION("Async operation") {
        useAsyncOperation(setup.server, methodId, true);
        CHECK_FALSE(getAsyncOperation(setup.server).has_value());
        auto future = services::callAsync(setup.client, objectId, methodId, {}, useFuture);
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds{1};
        std::optional<AsyncOperation> operation;
        while (!operation && std::chrono::steady_clock::now() < deadline) {
            operation = getAsyncOperation(setup.server);
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
        CHECK(operation.has_value());
        runAsyncOperation(setup.server, operation.value());
        setup.client.runIterate();
        const auto result = future.get();
        CHECK(result.statusCode().isGood());
        CHECK(result.outputArguments().at(0).to<uint64_t>() == getThreadId());
    }
#endif
}
#endif
#endif
