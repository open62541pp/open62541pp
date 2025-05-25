#pragma once

#include <memory>
#include <utility>

#include "open62541pp/detail/traits.hpp"

namespace opcua::detail {

template <typename T>
class UniqueOrRawPtr {
public:
    constexpr UniqueOrRawPtr() = default;

    explicit constexpr UniqueOrRawPtr(std::unique_ptr<T>&& ptr) noexcept
        : uniquePtr_{std::move(ptr)},
          rawPtr_{uniquePtr_.get()} {}

    explicit constexpr UniqueOrRawPtr(T* ptr) noexcept
        : uniquePtr_{nullptr},
          rawPtr_{ptr} {}

    constexpr UniqueOrRawPtr& operator=(std::unique_ptr<T>&& ptr) noexcept {
        uniquePtr_ = std::move(ptr);
        rawPtr_ = uniquePtr_.get();
        return *this;
    }

    constexpr UniqueOrRawPtr& operator=(T* ptr) noexcept {
        uniquePtr_ = nullptr;
        rawPtr_ = ptr;
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

    constexpr T* get() noexcept {  // NOLINT(*exception-escape)
        return rawPtr_;
    }

    constexpr const T* get() const noexcept {
        return rawPtr_;
    }

    constexpr bool operator==(T* ptr) const noexcept {
        return get() == ptr;
    }

    constexpr bool operator!=(T* ptr) const noexcept {
        return get() != ptr;
    }

private:
    std::unique_ptr<T> uniquePtr_{nullptr};
    T* rawPtr_{nullptr};
};

}  // namespace opcua::detail
