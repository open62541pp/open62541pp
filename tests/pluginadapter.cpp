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

    void clear(TestPlugin& plugin) const noexcept override {
        if (plugin.clear != nullptr) {
            plugin.clear(plugin.context);
        }
    }

    void clear(TestPlugin*& plugin) const noexcept override {
        if (plugin != nullptr) {
            clear(*plugin);
            delete plugin;  // NOLINT
            plugin = nullptr;
        }
    }
};

TEST_CASE("PluginAdapter") {
    auto adapter = std::make_unique<TestAdapter>();

    SUBCASE("Ref") {
        TestPlugin plugin{};
        plugin = adapter->create(false);
        CHECK(plugin.context != nullptr);
        CHECK(plugin.clear == nullptr);
        adapter->clear(plugin);
    }

    SUBCASE("Ref with owning adapter") {
        TestPlugin plugin{};
        auto* adapterPtr = adapter.release();
        plugin = adapterPtr->create(true);
        CHECK(plugin.context != nullptr);
        CHECK(plugin.clear != nullptr);
        adapterPtr->clear(plugin);
    }

    SUBCASE("Pointer with owning adapter") {
        TestPlugin* pluginPtr = new TestPlugin{};  // NOLINT
        auto* adapterPtr = adapter.release();
        *pluginPtr = adapterPtr->create(true);
        CHECK(pluginPtr->context != nullptr);
        CHECK(pluginPtr->clear != nullptr);
        adapterPtr->clear(pluginPtr);
    }
}

TEST_CASE("PluginAdapter helper") {
    auto adapter = std::make_unique<TestAdapter>();

    SUBCASE("assignPlugin ref") {
        TestPlugin plugin{};
        detail::assignPlugin(plugin, *adapter, false);
        CHECK(plugin.context != nullptr);
        CHECK(plugin.clear == nullptr);
        adapter->clear(plugin);
    }

    SUBCASE("assignPlugin ref with owning adapter") {
        TestPlugin plugin{};
        auto* adapterPtr = adapter.release();
        detail::assignPlugin(plugin, *adapterPtr, true);
        CHECK(plugin.context != nullptr);
        CHECK(plugin.clear != nullptr);
        adapterPtr->clear(plugin);
    }

    SUBCASE("assignPlugin pointer") {
        TestPlugin* pluginPtr = new TestPlugin{};
        detail::assignPlugin(pluginPtr, *adapter, false);
        CHECK(pluginPtr->context != nullptr);
        CHECK(pluginPtr->clear == nullptr);
        adapter->clear(pluginPtr);
    }

    // SUBCASE("assignPlugin pointer with owning adapter") {
    //     TestPlugin* pluginPtr = new TestPlugin{};
    //     auto* adapterPtr = adapter.release();
    //     detail::assignPlugin(pluginPtr, *adapterPtr, true);
    //     CHECK(pluginPtr->context != nullptr);
    //     CHECK(pluginPtr->clear != nullptr);
    //     adapter->clear(pluginPtr);
    // }
}
