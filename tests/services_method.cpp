#include <doctest/doctest.h>

#include "open62541pp/Config.h"
#include "open62541pp/services/Method.h"
#include "open62541pp/services/NodeManagement.h"  // addMethod

#include "helper/server_client_setup.h"

using namespace opcua;

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
        "Add",
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
