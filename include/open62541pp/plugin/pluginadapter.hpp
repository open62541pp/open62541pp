#pragma once

namespace opcua {

/**
 * Base class to implement plugin adapters.
 *
 * Plugin adapters will provide a different (modern) interface for open62541's native plugins.
 * For example AccessControlBase is the plugin adapter for UA_AccessControl.
 *
 * The native plugin is created via the PluginAdapter::create function.
 * Usually, the native plugin carries a `void* context` pointer which will refer back to the plugin
 * adapter. Either move the adapter ownership to the plugin with `ownsAdapter = true` or make sure
 * that the plugin adapter outlives the native plugin to avoid dangling pointers.
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

    virtual T create(bool ownsAdapter) = 0;
};

}  // namespace opcua
