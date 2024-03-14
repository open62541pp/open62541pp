#pragma once

namespace opcua {

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

    virtual void clear(T& plugin) noexcept = 0;

    void clear(T* plugin) noexcept {
        if (plugin != nullptr) {
            clear(*plugin);
        }
    }

    virtual T create() = 0;
};

}  // namespace opcua
