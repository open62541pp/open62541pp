#pragma once

#include <cassert>
#include <functional>  // invoke
#include <map>
#include <memory>
#include <mutex>
#include <type_traits>

namespace opcua::detail {

/**
 * Check if an object as a boolean `stale` flag.
 *
 * Context objects e.g. for MonitoredItems may have a lifetime and a callback might be called when
 * the item is deleted. So the context objects should be able to delete itself.
 * The context object passed to the callback as a parameter but the callback has no access to the
 * container owning this context object. Each context object could carry a reference to its owning
 * container and delete itself, but this can be problematic... Instead a simple flag `stale` can be
 * set if the item is deleted and should be removed from its container, the `ContextMap`.
 */
template <typename T, typename = void>
struct IsStaleable : std::false_type {};

template <typename T>
struct IsStaleable<T, std::void_t<decltype(std::declval<T>().stale)>>
    : std::is_same<decltype(std::declval<T>().stale), bool> {};

/**
 * Thread-safe map for context objects.
 * Context objects are reference as `void*` pointers in open62541 functions/callbacks. To prevent
 * pointer-invalidation, the objects are stored as unique pointers.
 * Stale objects will be removed when new objects are stored in the map.
 */
template <typename Key, typename Item>
class ContextMap {
public:
    /// Access or insert specified element
    Item* operator[](Key key) {
        eraseStale();
        auto lock = acquireLock();
        auto& item = map_[key];
        if (item == nullptr) {
            item = std::make_unique<Item>();  // allocate item if empty
        }
        return item.get();
    }

    /// Inserts an element or assigns to the current element if the key already exists
    Item* insert(Key key, std::unique_ptr<Item>&& item) {
        eraseStale();
        auto lock = acquireLock();
        return map_.insert_or_assign(key, std::move(item)).first->second.get();
    }

    size_t erase(Key key) {
        auto lock = acquireLock();
        return map_.erase(key);
    }

    size_t eraseStale() {
        const size_t count = map_.size();
        if constexpr (IsStaleable<Item>::value) {
            auto lock = acquireLock();
            for (auto it = map_.begin(); it != map_.end();) {
                if (it->second->stale) {
                    it = map_.erase(it);
                } else {
                    ++it;
                }
            }
        }
        return count - map_.size();
    }

    bool contains(Key key) const {
        auto lock = acquireLock();
        return map_.count(key) > 0;
    }

    const Item* find(Key key) const {
        auto lock = acquireLock();
        auto it = map_.find(key);
        if (it != map_.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    template <typename F>
    void iterate(F&& func) const {  // NOLINT(cppcoreguidelines-missing-std-forward)
        auto lock = acquireLock();
        for (const auto& pair : map_) {
            std::invoke(func, pair);
        }
    }

private:
    [[nodiscard]] auto acquireLock() const {
        return std::unique_lock(mutex_);
    }

    std::map<Key, std::unique_ptr<Item>> map_;
    mutable std::mutex mutex_;
};

}  // namespace opcua::detail
