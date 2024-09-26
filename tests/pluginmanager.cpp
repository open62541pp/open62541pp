#include <memory>  // make_unique

#include <doctest/doctest.h>

#include "open62541pp/plugin/pluginadapter.hpp"

#include "plugin/pluginmanager.hpp"

using namespace opcua;

class IntAdapter : public PluginAdapter<int> {
public:
    explicit IntAdapter(int value)
        : value_(value) {}

    void clear(int& /* unused */) noexcept override {}

    int create() override {
        return value_;
    }

private:
    int value_;
};

class IntAdapterDeleteInClear : public PluginAdapter<int> {
public:
    explicit IntAdapterDeleteInClear(int value)
        : value_(value) {}

    void clear(int& /* unused */) noexcept override {}

    void clear(int*& plugin) noexcept override {
        if (plugin != nullptr) {
            delete plugin;
            plugin = nullptr;
        }
    }

    int create() override {
        return value_;
    }

private:
    int value_;
};

TEST_CASE("PluginManager with references") {
    int plugin = 0;
    PluginManager<int> manager(plugin);
    manager.assign(std::make_unique<IntAdapter>(1));
    CHECK(plugin == 1);
    manager.assign(std::make_unique<IntAdapter>(2));
    CHECK(plugin == 2);
}

TEST_CASE("PluginManager with pointers") {
    int* plugin{nullptr};
    PluginManager<int*> manager(plugin);

    SUBCASE("Nullptr") {}

    SUBCASE("Allocated") {
        plugin = new int;
        *plugin = 0;
    }

    manager.assign(std::make_unique<IntAdapter>(1));
    CHECK(*plugin == 1);
    manager.assign(std::make_unique<IntAdapterDeleteInClear>(2));
    CHECK(*plugin == 2);
    delete plugin;
}
