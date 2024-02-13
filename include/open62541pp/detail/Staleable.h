#pragma once

#include <type_traits>

namespace opcua::detail {

/**
 * Object that can mark itself as stale (to be removed).
 *
 * Context objects e.g. for MonitoredItems may have a lifetime and a callback might be called when
 * the item is deleted. So the context objects should be able to delete itself.
 * The context object passed to the callback as a parameter but the callback has no access to the
 * container owning this context object. Each context object could carry a reference to its owning
 * container and delete itself, but this can be problematic... Instead a simple flag `stale` can be
 * set if the item is deleted and should be removed from its container, the `ContextMap`.
 */
struct Staleable {
    bool stale = false;  ///< Mark the object to be removed
};

/**
 * Check if an object as a boolean `stale` flag.
 */
template <typename T, typename = void>
struct IsStaleable : std::false_type {};

template <typename T>
struct IsStaleable<T, std::void_t<decltype(std::declval<T>().stale)>>
    : std::is_same<decltype(std::declval<T>().stale), bool> {};

}  // namespace opcua::detail
