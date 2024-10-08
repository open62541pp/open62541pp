#pragma once

#include <cassert>
#include <memory>
#include <type_traits>

namespace opcua::detail {

/**
 * Helper base class to derive a wrapper (e.g. `Server&`) from its underlying connection.
 * The single connection pointer member is the same as in the parent connection class. A custom
 * deleter is provided to prevent circular dependencies and double deletion.
 * Because the connection pointer is the first and single member of the wrapper class, the
 * connection pointer and the wrapper class are pointer-interconvertible (standard-layout required).
 */
template <typename WrapperType>
class ConnectionBase {
public:
    static_assert(sizeof(WrapperType) == sizeof(std::unique_ptr<ConnectionBase>));

    // unique_ptr<T> is standard layout but clang disagrees:
    // https://github.com/llvm/llvm-project/issues/53021
    // static_assert(std::is_standard_layout_v<WrapperType>);

    ConnectionBase() noexcept
        : connectionPtr_(this) {}

    ~ConnectionBase() = default;

    ConnectionBase(const ConnectionBase&) = delete;
    ConnectionBase(ConnectionBase&&) noexcept = delete;
    ConnectionBase& operator=(const ConnectionBase&) = delete;
    ConnectionBase& operator=(ConnectionBase&&) noexcept = delete;

    [[nodiscard]] WrapperType* wrapperPtr() noexcept {
        assert(connectionPtr_ != nullptr);
        // NOLINTNEXTLINE(bugprone-casting-through-void)
        return static_cast<WrapperType*>(static_cast<void*>(&connectionPtr_));
    };

    [[nodiscard]] WrapperType& wrapper() noexcept {
        return *wrapperPtr();
    }

private:
    struct NoDeleter {
        constexpr void operator()(ConnectionBase* /* unused */) noexcept {};
    };

    std::unique_ptr<ConnectionBase, NoDeleter> connectionPtr_;
};

}  // namespace opcua::detail
