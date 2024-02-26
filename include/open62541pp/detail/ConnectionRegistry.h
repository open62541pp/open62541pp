#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <mutex>
#include <optional>

namespace opcua::detail {

template <typename T>
class ConnectionRegistry {
public:
    using Handle = decltype(std::declval<T>().handle());
    using Connection = typename T::Connection;

    static void registerInstance(Handle handle, std::shared_ptr<Connection>& state) {
        assert(state != nullptr);
        std::lock_guard lock(instancesMutex);
        instances.insert_or_assign(handle, std::move(state));
        removeExpiredInstances();
    }

    [[nodiscard]] static std::optional<T> findInstance(const Handle handle) {
        std::lock_guard lock(instancesMutex);
        auto it = instances.find(handle);
        if (it == instances.end()) {
            return std::nullopt;
        }
        auto& state = it->second;
        if (state.expired()) {
            instances.erase(it);
            return std::nullopt;
        }
        return T(state.lock());  // requires constructor with std::shared_ptr<Connection>
    }

private:
    inline static std::map<Handle, std::weak_ptr<Connection>> instances;
    inline static std::mutex instancesMutex;

    static void removeExpiredInstances() {
        for (auto it = instances.begin(); it != instances.end();) {
            if (it->second.expired()) {
                it = instances.erase(it);
            } else {
                ++it;
            }
        }
    }
};

}  // namespace opcua::detail
