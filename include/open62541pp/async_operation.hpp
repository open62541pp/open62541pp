#pragma once

#include <cassert>
#include <cstddef>  // size_t
#include <functional>
#include <memory>
#include <utility>  // move, pair

#include "open62541pp/config.hpp"  // UAPP_HAS_ASYNC_OPERATIONS, UAPP_OPEN62541_VER_GE
#include "open62541pp/detail/async_operation_registry.hpp"
#include "open62541pp/detail/open62541/server.h"
#include "open62541pp/detail/types_handling.hpp"  // copyArray, deallocateArray
#include "open62541pp/span.hpp"
#include "open62541pp/typeregistry.hpp"  // getDataType
#include "open62541pp/types.hpp"  // StatusCode, Variant, DataValue
#include "open62541pp/wrapper.hpp"  // asWrapper, asNative

#if UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)

namespace opcua {

/**
 * @defgroup AsyncOperations Async operation completion (server-side)
 * Defer the completion of a server-side callback (method call, data source read/write) and
 * complete it later from any thread.
 *
 * Results are passed to `complete()` by value and handed over to the event loop thread of the
 * server, which writes them only if the operation is still pending. The server can cancel an
 * operation at any time (e.g. timeout or closed session), a result that arrives afterwards is
 * dropped.
 *
 * @note Requires open62541 v1.5 or later and `UA_MULTITHREADING >= 100`.
 * @see Async
 * @{
 */

/// Base class of AsyncCallOperation, AsyncReadOperation and AsyncWriteOperation.
/// Move-only. `complete()` of the derived classes can be called from any thread.
/// @note Requires open62541 v1.5 or later and `UA_MULTITHREADING >= 100`.
class AsyncOperationBase {
public:
    AsyncOperationBase(const AsyncOperationBase&) = delete;
    AsyncOperationBase& operator=(const AsyncOperationBase&) = delete;
    AsyncOperationBase(AsyncOperationBase&&) noexcept = default;
    AsyncOperationBase& operator=(AsyncOperationBase&&) noexcept = default;
    ~AsyncOperationBase() = default;

    /// Check if the server has cancelled the operation, e.g. due to a timeout, a closed session or
    /// a synchronous call that cannot wait for a deferred result.
    /// `complete()` has no effect afterwards.
    bool cancelled() const noexcept {
        return state_->cancelled();
    }

protected:
    AsyncOperationBase(
        UA_Server* server,
        detail::AsyncOperationRegistry& registry,
        std::shared_ptr<detail::AsyncOperationState> state,
        const void* identity
    ) noexcept
        : server_{server},
          registry_{&registry},
          state_{std::move(state)},
          identity_{identity} {}

    /// Hand `completion` over to the event loop thread of the server, where it runs only if the
    /// operation is still pending.
    void finish(detail::AsyncOperationRegistry::Completion completion) {
        if (!tryClaim()) {
            return;  // already cancelled or completed
        }
        if (!registry_->post(identity_, state_, std::move(completion))) {
            return;  // cancelled in the meantime
        }
        UA_Server_addTimedCallback(
            server(), &AsyncOperationBase::drain, registry_, UA_DateTime_nowMonotonic(), nullptr
        );
    }

    UA_Server* server() const noexcept {
        return server_;
    }

private:
    static void drain(UA_Server* /* server */, void* registry) {
        static_cast<detail::AsyncOperationRegistry*>(registry)->drain();
    }

    bool tryClaim() noexcept {
        return state_->tryComplete();
    }

    UA_Server* server_;
    detail::AsyncOperationRegistry* registry_;
    std::shared_ptr<detail::AsyncOperationState> state_;
    const void* identity_;
};

/// Handle to a pending async method-call completion.
/// Returned by AsyncMethodContext::defer().
/// @note Requires open62541 v1.5 or later and `UA_MULTITHREADING >= 100`.
class AsyncCallOperation : public AsyncOperationBase {
public:
    AsyncCallOperation(
        UA_Server* server,
        detail::AsyncOperationRegistry& registry,
        std::shared_ptr<detail::AsyncOperationState> state,
        UA_Variant* output,
        size_t outputSize
    ) noexcept
        : AsyncOperationBase{server, registry, std::move(state), output},
          output_{output},
          outputSize_{outputSize} {}

    /// Number of output arguments of the method.
    size_t outputSize() const noexcept {
        return outputSize_;
    }

    /// Deliver the output arguments (copied) and complete the call with `status`.
    /// Fewer arguments than outputSize() are allowed (the rest stays empty), more are an error.
    /// No-op if the operation was cancelled.
    void complete(Span<const Variant> output, StatusCode status = UA_STATUSCODE_GOOD) {
        if (output.size() > outputSize_) {
            complete(StatusCode{UA_STATUSCODE_BADINTERNALERROR});
            return;
        }
        const size_t size = output.size();
        const UA_DataType* type = &getDataType<Variant>();
        // deep copy now: `output` may go away as soon as this function returns
        const std::shared_ptr<UA_Variant> copy{
            detail::copyArray(asNative(output.data()), size, *type),
            [size, type](UA_Variant* ptr) { detail::deallocateArray(ptr, size, *type); }
        };
        finish([server = server(), out = output_, copy, size, status] {
            const Span<UA_Variant> target{out, size};
            const Span<UA_Variant> source{copy.get(), size};
            for (size_t i = 0; i < size; ++i) {
                UA_Variant_clear(&target[i]);
                target[i] = source[i];
                source[i] = UA_Variant{};
            }
            UA_Server_setAsyncCallMethodResult(server, out, status.get());
        });
    }

    /// Complete the call with `status` and no output arguments.
    void complete(StatusCode status = UA_STATUSCODE_GOOD) {
        finish([server = server(), out = output_, status] {
            UA_Server_setAsyncCallMethodResult(server, out, status.get());
        });
    }

private:
    UA_Variant* output_;
    size_t outputSize_;
};

/// Handle to a pending async variable-read completion.
/// Returned by AsyncReadContext::defer().
/// @note Requires open62541 v1.5 or later and `UA_MULTITHREADING >= 100`.
class AsyncReadOperation : public AsyncOperationBase {
public:
    AsyncReadOperation(
        UA_Server* server,
        detail::AsyncOperationRegistry& registry,
        std::shared_ptr<detail::AsyncOperationState> state,
        UA_DataValue* result
    ) noexcept
        : AsyncOperationBase{server, registry, std::move(state), result},
          result_{result} {}

    /// Deliver the result: value, status and timestamps all live in the DataValue.
    /// No-op if the operation was cancelled.
    void complete(DataValue value) {
        finish([server = server(), result = result_, v = std::move(value)]() mutable {
            asWrapper<DataValue>(*result) = std::move(v);
            UA_Server_setAsyncReadResult(server, result);
        });
    }

private:
    UA_DataValue* result_;
};

/// Handle to a pending async variable-write completion.
/// Returned by AsyncWriteContext::defer().
/// @note Requires open62541 v1.5 or later and `UA_MULTITHREADING >= 100`.
class AsyncWriteOperation : public AsyncOperationBase {
public:
    AsyncWriteOperation(
        UA_Server* server,
        detail::AsyncOperationRegistry& registry,
        std::shared_ptr<detail::AsyncOperationState> state,
        const UA_DataValue* value
    ) noexcept
        : AsyncOperationBase{server, registry, std::move(state), value},
          value_{value} {}

    /// Deliver the result. No-op if the operation was cancelled.
    void complete(StatusCode status = UA_STATUSCODE_GOOD) {
        finish([server = server(), value = value_, status] {
            UA_Server_setAsyncWriteResult(server, value, status.get());
        });
    }

private:
    const UA_DataValue* value_;
};

}  // namespace opcua

namespace opcua::detail {

/// Base class of AsyncMethodContext, AsyncReadContext and AsyncWriteContext.
/// Registers the pending operation and guards against calling defer() twice.
class AsyncContextBase {
public:
    AsyncContextBase(const AsyncContextBase&) = delete;
    AsyncContextBase& operator=(const AsyncContextBase&) = delete;
    AsyncContextBase(AsyncContextBase&&) = delete;
    AsyncContextBase& operator=(AsyncContextBase&&) = delete;

protected:
    AsyncContextBase(
        UA_Server* server, AsyncOperationRegistry& registry, const void* identity
    ) noexcept
        : server_{server},
          registry_{&registry},
          identity_{identity} {}

    ~AsyncContextBase() = default;

    std::shared_ptr<AsyncOperationState> deferImpl() {
        assert(!deferred_ && "defer() called twice");
        deferred_ = true;
        return registry_->add(identity_);
    }

    UA_Server* server() const noexcept {
        return server_;
    }

    AsyncOperationRegistry& registry() const noexcept {
        return *registry_;
    }

private:
    UA_Server* server_;
    AsyncOperationRegistry* registry_;
    const void* identity_;
    bool deferred_ = false;
};

}  // namespace opcua::detail

namespace opcua {

/// Context passed to method callbacks to defer the completion of the method call.
/// @see MethodCallback
/// @note Requires open62541 v1.5 or later and `UA_MULTITHREADING >= 100`.
class AsyncMethodContext : public detail::AsyncContextBase {
public:
    AsyncMethodContext(
        UA_Server* server,
        detail::AsyncOperationRegistry& registry,
        UA_Variant* output,
        size_t outputSize
    ) noexcept
        : AsyncContextBase{server, registry, output},
          output_{output},
          outputSize_{outputSize} {}

    /// Defer the completion of the method call.
    /// Move the returned operation to the thread that finishes the work and return the status
    /// (`UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY`) from the method callback.
    [[nodiscard]] std::pair<AsyncCallOperation, StatusCode> defer() {
        AsyncCallOperation operation{server(), registry(), deferImpl(), output_, outputSize_};
        return {std::move(operation), UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY};
    }

private:
    UA_Variant* output_;
    size_t outputSize_;
};

/// Context passed to `DataSource<true, ...>::read()` to defer the completion of the read.
/// @note Requires open62541 v1.5 or later and `UA_MULTITHREADING >= 100`.
class AsyncReadContext : public detail::AsyncContextBase {
public:
    AsyncReadContext(
        UA_Server* server, detail::AsyncOperationRegistry& registry, UA_DataValue* result
    ) noexcept
        : AsyncContextBase{server, registry, result},
          result_{result} {}

    /// Defer the completion of the read.
    /// Move the returned operation to the thread that finishes the work and return the status
    /// from `read()`.
    [[nodiscard]] std::pair<AsyncReadOperation, StatusCode> defer() {
        AsyncReadOperation operation{server(), registry(), deferImpl(), result_};
        return {std::move(operation), UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY};
    }

private:
    UA_DataValue* result_;
};

/// Context passed to `DataSource<..., true>::write()` to defer the completion of the write.
/// @note Requires open62541 v1.5 or later and `UA_MULTITHREADING >= 100`.
class AsyncWriteContext : public detail::AsyncContextBase {
public:
    AsyncWriteContext(
        UA_Server* server, detail::AsyncOperationRegistry& registry, const UA_DataValue* value
    ) noexcept
        : AsyncContextBase{server, registry, value},
          value_{value} {}

    /// Defer the completion of the write.
    /// Move the returned operation to the thread that finishes the work and return the status
    /// from `write()`.
    [[nodiscard]] std::pair<AsyncWriteOperation, StatusCode> defer() {
        AsyncWriteOperation operation{server(), registry(), deferImpl(), value_};
        return {std::move(operation), UA_STATUSCODE_GOODCOMPLETESASYNCHRONOUSLY};
    }

private:
    const UA_DataValue* value_;
};

/**
 * @}
 */

}  // namespace opcua

#endif  // UAPP_HAS_ASYNC_OPERATIONS && UAPP_OPEN62541_VER_GE(1, 5)
