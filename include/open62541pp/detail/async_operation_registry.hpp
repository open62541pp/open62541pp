#pragma once

#include <atomic>
#include <cstddef>  // size_t
#include <cstdint>  // uint8_t
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>  // move
#include <vector>

#include "open62541pp/config.hpp"  // UAPP_HAS_ASYNC_OPERATIONS, UAPP_OPEN62541_VER_GE

#if UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)

namespace opcua::detail {

/// State of a pending async operation, shared between the operation handle and the registry.
/// The state object identifies the operation, not the raw pointer used by open62541, which can be
/// reused after a cancellation.
class AsyncOperationState {
public:
    /// Pending -> Completed. Fails if the operation is already completed or cancelled.
    bool tryComplete() noexcept {
        auto expected = Status::Pending;
        return status_.compare_exchange_strong(
            expected, Status::Completed, std::memory_order_acq_rel
        );
    }

    /// Any -> Cancelled. Wins over an already queued completion, its result is dropped.
    void cancel() noexcept {
        status_.store(Status::Cancelled, std::memory_order_release);
    }

    /// True if the server cancelled the operation. Not set by a regular completion.
    bool cancelled() const noexcept {
        return status_.load(std::memory_order_acquire) == Status::Cancelled;
    }

private:
    enum class Status : std::uint8_t { Pending, Completed, Cancelled };

    std::atomic<Status> status_{Status::Pending};
};

/// Track the pending async operations of a server and hand completions over to its event loop.
/// Owned by ServerContext, the asyncOperationCancelCallback of the server forwards to cancel().
class AsyncOperationRegistry {
public:
    using Completion = std::function<void()>;

    [[nodiscard]] std::shared_ptr<AsyncOperationState> add(const void* identity) {
        auto state = std::make_shared<AsyncOperationState>();
        const std::lock_guard lock{mutex_};
        map_[identity] = state;
        return state;
    }

    /// Called from the server's asyncOperationCancelCallback.
    void cancel(const void* identity) {
        std::shared_ptr<AsyncOperationState> state;
        {
            const std::lock_guard lock{mutex_};
            const auto it = map_.find(identity);
            if (it == map_.end()) {
                return;
            }
            state = std::move(it->second);
            map_.erase(it);
        }
        state->cancel();
    }

    /// Queue a completion if `state` is still the registered operation for `identity`.
    /// Can be called from any thread. Returns false if the operation is stale (cancelled or
    /// replaced), the completion is dropped then.
    bool post(
        const void* identity,
        const std::shared_ptr<AsyncOperationState>& state,
        Completion completion
    ) {
        const std::lock_guard lock{mutex_};
        const auto it = map_.find(identity);
        if (it == map_.end() || it->second != state) {
            return false;
        }
        pending_.push_back({identity, state, std::move(completion)});
        return true;
    }

    /// Run the queued completions of operations that are still registered, drop the others.
    /// Must be called on the event loop thread of the server, where cancellations and new
    /// operations are processed as well. Completions run without holding the registry lock.
    void drain() {
        std::vector<Pending> batch;
        {
            const std::lock_guard lock{mutex_};
            batch.swap(pending_);
        }
        for (auto& item : batch) {
            {
                const std::lock_guard lock{mutex_};
                const auto it = map_.find(item.identity);
                if (it == map_.end() || it->second != item.state) {
                    continue;  // cancelled or replaced meanwhile
                }
                map_.erase(it);
            }
            item.completion();
        }
    }

    size_t size() const {
        const std::lock_guard lock{mutex_};
        return map_.size();
    }

private:
    struct Pending {
        const void* identity;
        std::shared_ptr<AsyncOperationState> state;
        Completion completion;
    };

    mutable std::mutex mutex_;
    std::unordered_map<const void*, std::shared_ptr<AsyncOperationState>> map_;
    std::vector<Pending> pending_;
};

}  // namespace opcua::detail

#endif  // UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)
