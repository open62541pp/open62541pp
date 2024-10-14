#include <memory>

#include <doctest/doctest.h>

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

    SUBCASE("Borrow") {
        plugin = adapter->create(false);
        CHECK(plugin.context != nullptr);
        CHECK(plugin.clear == nullptr);
    }

    SUBCASE("Owning") {
        plugin = adapter->create(true);
        adapter.release();
        CHECK(plugin.context != nullptr);
        CHECK(plugin.clear != nullptr);
    }

    if (plugin.clear != nullptr) {
        plugin.clear(plugin.context);
    }
}
