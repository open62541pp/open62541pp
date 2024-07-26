#pragma once

#include <memory>
#include <type_traits>
#include <variant>

#include "open62541pp/detail/traits.h"  // Overload
#include "open62541pp/plugins/PluginAdapter.h"

namespace opcua {

template <typename T>
class PluginManager {
public:
    using PluginType = std::remove_pointer_t<T>;
    using AdapterType = PluginAdapter<PluginType>;

    explicit constexpr PluginManager(T& plugin)
        : managed_(plugin) {}

    void assign(std::unique_ptr<AdapterType> adapter) {
        if (adapter != nullptr) {
            adapter_ = std::move(adapter);
            assign();
        }
    }

    // TODO: deprecate, pointer might get invalided
    void assign(AdapterType* adapter) {
        if (adapter != nullptr) {
            adapter_ = adapter;
            assign();
        }
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, AdapterType*>>>
    void assign(U&& adapter) {
        assign(std::make_unique<U>(std::forward<U>(adapter)));
    }

private:
    void assign() {
        assert(adapter() != nullptr);
        adapter()->clear(managed_);
        if constexpr (std::is_pointer_v<T>) {
            adapter()->clear(plugin_.get());
            plugin_ = std::make_unique<PluginType>(adapter()->create());
            managed_ = plugin_.get();
        } else {
            managed_ = adapter()->create();
        }
    }

    AdapterType* adapter() noexcept {
        return std::visit(
            detail::Overload{
                [](AdapterType* ptr) { return ptr; },
                [](std::unique_ptr<AdapterType>& ptr) { return ptr.get(); },
            },
            adapter_
        );
    }

    T& managed_;
    std::variant<AdapterType*, std::unique_ptr<AdapterType>> adapter_;
    std::unique_ptr<PluginType> plugin_;  // store the plugin here if T is a pointer
};

}  // namespace opcua
