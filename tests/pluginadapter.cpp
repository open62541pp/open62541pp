#include <memory>

#include <catch2/catch_test_macros.hpp>

#include "open62541pp/plugin/pluginadapter.hpp"

using namespace opcua;

struct TestPlugin {
    void* context;
    void (*clear)(void* context);
};

class TestAdapter : public PluginAdapter<TestPlugin> {
public:
    TestPlugin create(bool ownsAdapter) override {
        TestPlugin plugin{};
        plugin.context = this;
        if (ownsAdapter) {
            plugin.clear = [](void* context) {
                if (context != nullptr) {
                    delete static_cast<TestAdapter*>(context);  // NOLINT
                    context = nullptr;
                }
            };
        }
        return plugin;
    }
};

TEST_CASE("PluginAdapter") {
    auto adapter = std::make_unique<TestAdapter>();
    TestPlugin plugin{};

    SECTION("Borrow") {
        plugin = adapter->create(false);
        CHECK(plugin.context != nullptr);
        CHECK(plugin.clear == nullptr);
    }

    SECTION("Owning") {
        plugin = adapter->create(true);
        adapter.release();
        CHECK(plugin.context != nullptr);
        CHECK(plugin.clear != nullptr);
    }

    if (plugin.clear != nullptr) {
        plugin.clear(plugin.context);
    }
}
