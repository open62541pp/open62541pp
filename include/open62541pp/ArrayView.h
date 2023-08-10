#pragma once

#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>  // swap

namespace opcua {

/**
 * View to a contiguous sequence of objects, similar to `std::span` in C++20.
 *
 * ArrayViews are used to return open62541 arrays without copy and to use them with the standard
 * library algorithms. The view just holds two members: the pointer to `T` and the size, so it's
 * lightweight and trivially copyable.
 *
 * @tparam T Type of the array object, use `const T` for an immutable view
 *
 * @see https://en.cppreference.com/w/cpp/container/span
 */
template <typename T>
class ArrayView {
public:
    // clang-format off
    using element_type           = T;
    using value_type             = std::remove_cv_t<T>;
    using size_type              = size_t;
    using difference_type        = std::ptrdiff_t;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = T&;
    using const_reference        = const T&;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    // clang-format on

    constexpr ArrayView() noexcept = default;

    constexpr ArrayView(element_type* data, size_t size) noexcept
        : size_(size),
          data_(data) {}

    /**
     * Implicit constructor from a container like `std::array` or `std::vector`.
     */
    template <typename Container>
    constexpr ArrayView(Container& container) noexcept  // NOLINT
        : ArrayView(container.data(), container.size()) {}

    /**
     * Implicit constructor from a container like `std::array` or `std::vector` (const).
     */
    template <typename Container>
    constexpr ArrayView(const Container& container) noexcept  // NOLINT
        : ArrayView(container.data(), container.size()) {}

    /**
     * Implicit constructor from an initializer list.
     *
     * Only safe to use if `std::initializer_list` itself outlives the ArrayView:
     * @code{.cpp}
     * void takeView(ArrayView<const int> values);
     * // ok
     * takeView({1, 2, 3});
     * // not ok
     * ArrayView<const int> values = {1, 2, 3};
     * takeView(values);
     * @endcode
     */
    constexpr ArrayView(std::initializer_list<value_type> values) noexcept  // NOLINT
        : ArrayView(values.begin(), values.size()) {}

    constexpr void swap(ArrayView& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
    }

    [[nodiscard]] constexpr size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] constexpr pointer data() const noexcept {
        return data_;
    }

    [[nodiscard]] constexpr reference operator[](size_t index) const noexcept {
        return data()[index];
    }

    [[nodiscard]] constexpr reference front() const noexcept {
        return *data();
    }

    [[nodiscard]] constexpr reference back() const noexcept {
        return *(data() + size() - 1);
    }

    [[nodiscard]] constexpr iterator begin() const noexcept {
        return {data()};
    }

    [[nodiscard]] constexpr iterator end() const noexcept {
        return {data() + size()};
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
        return {data()};
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept {
        return {data() + size()};
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept {
        return reverse_iterator(end());
    }

    [[nodiscard]] constexpr reverse_iterator rend() const noexcept {
        return reverse_iterator(begin());
    }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    /// Obtain a view over `count` elements of this ArrayView starting at offset `offset`.
    [[nodiscard]] constexpr ArrayView subview(
        size_t offset, size_t count = std::numeric_limits<std::size_t>::max()
    ) const noexcept {
        if (offset >= size()) {
            return {};
        }
        if (count > size() - offset) {
            count = size() - offset;
        }
        return {data() + offset, count};
    }

    /// Obtain a view over the first `count` elements of this ArrayView.
    [[nodiscard]] constexpr ArrayView first(size_t count) const noexcept {
        if (count >= size()) {
            return *this;
        }
        return {data(), count};
    }

    /// Obtain a view over the last `count` elements of this ArrayView.
    [[nodiscard]] constexpr ArrayView last(size_t count) const noexcept {
        if (count >= size()) {
            return *this;
        }
        return {data() + (size() - count), count};
    }

private:
    size_t size_{0};
    T* data_{nullptr};
};

/* -------------------------------------- Deduction guides -------------------------------------- */

template <typename Container>
ArrayView(Container&) -> ArrayView<typename Container::value_type>;

template <typename Container>
ArrayView(const Container&) -> ArrayView<const typename Container::value_type>;

}  // namespace opcua
