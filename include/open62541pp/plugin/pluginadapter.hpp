#pragma once

namespace opcua {

/**
 * Base class to implement plugin adapters.
 *
 * Plugin adapters will provide a different (modern) interface for open62541's native plugins.
 * For example AccessControlBase is the plugin adapter for UA_AccessControl.
 *
 * The native plugin is created via the PluginAdapter::create function.
 * The PluginAdapter::clear function is used to clear the created plugin after usage.
 * Usually the native plugin carries a `void* context` pointer which will refer back to the plugin
 * adapter. Make sure that the plugin adapter outlives the native plugin to avoid dangling pointers.
 *
 * @tparam T Type of the native plugin, e.g. UA_AccessControl
 */
template <typename T>
class PluginAdapter {
public:
    using PluginType = T;

    PluginAdapter() = default;
    virtual ~PluginAdapter() = default;

    PluginAdapter(const PluginAdapter&) = default;
    PluginAdapter(PluginAdapter&&) noexcept = default;
    PluginAdapter& operator=(const PluginAdapter&) = default;
    PluginAdapter& operator=(PluginAdapter&&) noexcept = default;

    virtual T create() = 0;

    virtual void clear(T& plugin) noexcept = 0;

    virtual void clear(T*& plugin) noexcept {
        if (plugin != nullptr) {
            clear(*plugin);
        }
    }

    void assign(T& plugin) {
        clear(plugin);
        plugin = create();
    }

    void assign(T*& plugin) {
        if (plugin == nullptr) {
            plugin = new T{};  // NOLINT
        } else {
            clear(plugin);
        }
        *plugin = create();
    }
};

}  // namespace opcua
