#pragma once

#include <memory>
#include <utility>
#include <variant>

#include "open62541pp/detail/traits.hpp"

namespace opcua::detail {

template <typename T>
class UniqueOrRawPtr {
public:
    constexpr UniqueOrRawPtr() = default;

    explicit constexpr UniqueOrRawPtr(std::unique_ptr<T>&& ptr) noexcept
        : ptr_(std::move(ptr)) {}

    explicit constexpr UniqueOrRawPtr(T* ptr) noexcept
        : ptr_(ptr) {}

    constexpr UniqueOrRawPtr& operator=(std::unique_ptr<T>&& ptr) noexcept {
        ptr_ = std::move(ptr);
        return *this;
    }

    constexpr UniqueOrRawPtr& operator=(T* ptr) noexcept {
        ptr_ = ptr;
        return *this;
    }

    constexpr T& operator*() noexcept {
        return *get();
    }

    constexpr const T& operator*() const noexcept {
        return *get();
    }

    constexpr T* operator->() noexcept {
        return get();
    }

    constexpr const T* operator->() const noexcept {
        return get();
    }

    constexpr T* get() noexcept {
        return std::visit(
            Overload{
                [](T* ptr) { return ptr; },
                [](std::unique_ptr<T>& ptr) { return ptr.get(); },
            },
            ptr_
        );
    }

    constexpr const T* get() const noexcept {
        return (const_cast<UniqueOrRawPtr*>(this)->get());  // NOLINT(*const-cast)
    }

    constexpr bool operator==(T* ptr) const noexcept {
        return get() == ptr;
    }

    constexpr bool operator!=(T* ptr) const noexcept {
        return get() != ptr;
    }

private:
    std::variant<T*, std::unique_ptr<T>> ptr_{nullptr};
};

}  // namespace opcua::detail
