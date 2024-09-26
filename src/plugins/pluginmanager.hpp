#pragma once

#include <cassert>
#include <memory>
#include <type_traits>
#include <variant>

#include "open62541pp/detail/traits.hpp"  // Overload
#include "open62541pp/plugins/pluginadapter.hpp"

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
            create();
        }
    }

    // TODO: deprecate, pointer might get invalided
    void assign(AdapterType* adapter) {
        if (adapter != nullptr) {
            adapter_ = adapter;
            create();
        }
    }

    template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, AdapterType*>>>
    void assign(U&& adapter) {
        assign(std::make_unique<U>(std::forward<U>(adapter)));
    }

private:
    void create() {
        assert(adapter() != nullptr);
        clear();
        managed() = adapter()->create();
    }

    void clear() noexcept {
        if (adapter() != nullptr) {
            adapter()->clear(managed_);
        }
    }

    PluginType& managed() {
        if constexpr (std::is_pointer_v<T>) {
            if (managed_ == nullptr) {
                managed_ = std::make_unique<PluginType>(PluginType{}).release();
            }
            return *managed_;
        } else {
            return managed_;
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
};

}  // namespace opcua
