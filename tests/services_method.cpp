#include <doctest/doctest.h>

#include "open62541pp/config.hpp"
#include "open62541pp/services/method.hpp"
#include "open62541pp/services/nodemanagement.hpp"  // addMethod

#include "helper/server_client_setup.hpp"

using namespace opcua;

#ifdef UA_ENABLE_METHODCALLS
TEST_CASE_TEMPLATE("Method service set", T, Server, Client, Async<Client>) {
    ServerClientSetup setup;
    setup.client.connect(setup.endpointUrl);
    auto& connection = setup.getInstance<T>();

    const NodeId objectsId{ObjectId::ObjectsFolder};
    const NodeId methodId{1, 1000};

    bool throwException = false;
    REQUIRE(services::addMethod(
        setup.server,
        objectsId,
        methodId,
        "Add",
        [&](Span<const Variant> inputs, Span<Variant> outputs) {
            if (throwException) {
                throw BadStatus(UA_STATUSCODE_BADUNEXPECTEDERROR);
            }
            const auto a = inputs[0].scalar<int32_t>();
            const auto b = inputs[1].scalar<int32_t>();
            outputs[0].setScalarCopy(a + b);
        },
        {
            Argument("a", {"en-US", "first number"}, DataTypeId::Int32, ValueRank::Scalar),
            Argument("b", {"en-US", "second number"}, DataTypeId::Int32, ValueRank::Scalar),
        },
        {
            Argument("sum", {"en-US", "sum of both numbers"}, DataTypeId::Int32, ValueRank::Scalar),
        },
        MethodAttributes{},
        ReferenceTypeId::HasComponent
    ));

    auto call = [&](auto&&... args) {
        if constexpr (isAsync<T>) {
            auto future = services::callAsync(args..., useFuture);
            setup.client.runIterate();
            return future.get();
        } else {
            return services::call(args...);
        }
    };

    SUBCASE("Check result") {
        const CallMethodResult result = call(
            connection,
            objectsId,
            methodId,
            Span<const Variant>{
                Variant::fromScalar(int32_t{1}),
                Variant::fromScalar(int32_t{2}),
            }
        );
        CHECK(result.statusCode().isGood());
        CHECK(result.outputArguments().size() == 1);
        CHECK(result.outputArguments()[0].scalar<int32_t>() == 3);
    }

    SUBCASE("Propagate exception") {
        throwException = true;
        const CallMethodResult result = call(
            connection,
            objectsId,
            methodId,
            Span<const Variant>{
                Variant::fromScalar(int32_t{1}),
                Variant::fromScalar(int32_t{2}),
            }
        );
        CHECK(result.statusCode() == UA_STATUSCODE_BADUNEXPECTEDERROR);
    }

    SUBCASE("Invalid input arguments") {
        const CallMethodResult result = call(
            connection,
            objectsId,
            methodId,
            Span<const Variant>{
                Variant::fromScalar(true),
                Variant::fromScalar(11.11f),
            }
        );
        CHECK(result.statusCode() == UA_STATUSCODE_BADINVALIDARGUMENT);
    }

    SUBCASE("Missing arguments") {
        const CallMethodResult result = call(
            connection, objectsId, methodId, Span<const Variant>{}
        );
        CHECK(result.statusCode() == UA_STATUSCODE_BADARGUMENTSMISSING);
    }

    SUBCASE("Too many arguments") {
        const CallMethodResult result = call(
            connection,
            objectsId,
            methodId,
            Span<const Variant>{
                Variant::fromScalar(int32_t{1}),
                Variant::fromScalar(int32_t{2}),
                Variant::fromScalar(int32_t{3}),
            }
        );
        CHECK(result.statusCode() == UA_STATUSCODE_BADTOOMANYARGUMENTS);
    }
}
#endif
