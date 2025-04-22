#include <string>
#include <utility>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include "open62541pp/client.hpp"
#include "open62541pp/datatype.hpp"
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/server.hpp"

using namespace opcua;

TEMPLATE_TEST_CASE("Config", "", ClientConfig, ServerConfig) {
    using NativeType = typename TestType::NativeType;

    SECTION("Default constructor") {
        TestType config;
    }
    SECTION("Construct from native") {
        NativeType native{};
        TestType config{std::move(native)};
    }
    SECTION("Move constructor") {
        TestType other;
        TestType config{std::move(other)};
    }
    SECTION("Move assignment") {
        TestType other;
        TestType config = std::move(other);
    }

    TestType config;

    SECTION("setLogger") {
        static size_t counter = 0;
        static LogLevel lastLogLevel{};
        static LogCategory lastLogCategory{};
        static std::string lastMessage{};

        config.setLogger([&](LogLevel level, LogCategory category, std::string_view message) {
            counter++;
            lastLogLevel = level;
            lastLogCategory = category;
            lastMessage = message;
        });

        // passing a nullptr should do nothing
        config.setLogger(nullptr);

        UA_LOG_INFO(detail::getLogger(config.handle()), UA_LOGCATEGORY_USERLAND, "Message");
        CHECK(counter == 1);
        CHECK(lastLogLevel == LogLevel::Info);
        CHECK(lastLogCategory == LogCategory::Userland);
        CHECK(lastMessage == "Message");
    }

    SECTION("addCustomDataTypes") {
        CHECK(config->customDataTypes == nullptr);

        config.addCustomDataTypes({
            DataType(UA_TYPES[UA_TYPES_STRING]),
            DataType(UA_TYPES[UA_TYPES_INT32]),
        });
        CHECK(config->customDataTypes != nullptr);
        CHECK(config->customDataTypes->next == nullptr);
        CHECK(config->customDataTypes->typesSize == 2);
        CHECK(config->customDataTypes->types != nullptr);
        CHECK((config->customDataTypes->types[0] == UA_TYPES[UA_TYPES_STRING]));
        CHECK((config->customDataTypes->types[1] == UA_TYPES[UA_TYPES_INT32]));
    }

    SECTION("handle") {
        CHECK(config.handle() != nullptr);
        CHECK(std::as_const(config).handle() != nullptr);
    }
}

TEMPLATE_TEST_CASE("Connection", "", Client, Server) {
    // TODO: provide type alias NativeType
    using NativeType = std::remove_pointer_t<decltype(std::declval<TestType>().handle())>;

    SECTION("Default constructor") {
        TestType connection;
    }
    SECTION("Move constructor") {
        TestType other;
        TestType connection{std::move(other)};
    }
    SECTION("Move assignment") {
        TestType other;
        TestType connection = std::move(other);
    }

    TestType connection;

    SECTION("config") {
        auto* config = detail::getConfig(connection.handle());
        CHECK(connection.config().handle() == config);
        CHECK(std::as_const(connection).config().handle() == config);
    }

    SECTION("handle") {
        CHECK(connection.handle() != nullptr);
        CHECK(std::as_const(connection).handle() != nullptr);
    }

    SECTION("Equality operators") {
        TestType other;
        CHECK(connection == connection);
        CHECK(connection != other);
        CHECK(other == other);
    }

    SECTION("Utility functions") {
        NativeType* native = connection.handle();
        NativeType* nativeNull{nullptr};

        CHECK(detail::getConfig(nativeNull) == nullptr);
        CHECK(detail::getConfig(native) != nullptr);
        CHECK(detail::getConfig(native) == connection.config().handle());

        CHECK(detail::getLogger(detail::getConfig(native)) != nullptr);

        CHECK(detail::getContext(nativeNull) == nullptr);
        CHECK(detail::getContext(native) != nullptr);
        CHECK(detail::getContext(native) == &detail::getContext(connection));

        CHECK(detail::getExceptionCatcher(nativeNull) == nullptr);
        CHECK(detail::getExceptionCatcher(native) != nullptr);
        CHECK(detail::getExceptionCatcher(native) == &detail::getExceptionCatcher(connection));

        CHECK(detail::getHandle(connection) == connection.handle());
    }
}

TEMPLATE_TEST_CASE("Connection asWrapper", "", Client, Server) {
    using NativeType = std::remove_pointer_t<decltype(std::declval<TestType>().handle())>;

    TestType connection;
    NativeType* native = connection.handle();
    NativeType* nativeNull{nullptr};

    CHECK(asWrapper(nativeNull) == nullptr);
    CHECK(asWrapper(native) != nullptr);
    CHECK(asWrapper(native) == &connection);
    CHECK(asWrapper(native)->handle() == native);

    SECTION("Move construct") {
        TestType connectionMoved{std::move(connection)};
        CHECK(asWrapper(native) == &connectionMoved);
    }
    SECTION("Move assignment") {
        TestType connectionMoved;
        connectionMoved = std::move(connection);
        CHECK(asWrapper(native) == &connectionMoved);
    }
}
