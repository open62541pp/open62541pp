#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <type_traits>

#include "open62541pp/detail/Staleable.h"

namespace opcua::detail {

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

    /// Acquire the lock/mutex for unique access to the underlying map.
    [[nodiscard]] auto acquireLock() const {
        return std::lock_guard(mutex_);
    }

    /// Get access to the underlying map. Use `acquireLock` before any operation.
    const auto& underlying() const noexcept {
        return map_;
    }

private:
    std::map<Key, std::unique_ptr<Item>> map_;
    mutable std::mutex mutex_;
};

}  // namespace opcua::detail
