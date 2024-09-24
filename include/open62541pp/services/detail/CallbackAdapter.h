#pragma once

#include <functional>

#include "open62541pp/detail/ExceptionCatcher.h"

namespace opcua::services::detail {

/**
 * Helper class to map open62541 / C-style callbacks to `std::function` objects.
 * The the concrete adapter should:
 * - store the `std::function` objects
 * - implement static methods of the native open62541 callbacks
 * - pass the pointer to itself as the callback context
 */
struct CallbackAdapter {
    // Optionally provide exception catcher, otherwise exceptions will be ignored
    opcua::detail::ExceptionCatcher* catcher = nullptr;

    template <typename F, typename... Args>
    void invoke(F&& callback, Args&&... args) const noexcept {
        if (callback != nullptr) {
            if (catcher != nullptr) {
                catcher->invoke(std::forward<F>(callback), std::forward<Args>(args)...);
            } else {
                try {
                    std::invoke(std::forward<F>(callback), std::forward<Args>(args)...);
                } catch (...) {  // NOLINT(bugprone-empty-catch)
                    // ignore exceptions
                }
            }
        }
    }
};

}  // namespace opcua::services::detail
