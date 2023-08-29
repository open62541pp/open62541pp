#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>  // swap
#include <vector>

namespace opcua {

/**
 * View to a contiguous sequence of objects, similar to `std::span` in C++20.
 *
 * Spans are used to pass and return open62541 arrays without copy and to use them with the standard
 * library algorithms. The view just holds two members: the pointer to `T` and the size, so it's
 * lightweight and trivially copyable.
 *
 * @tparam T Type of the array object, use `const T` for an immutable view
 *
 * @see https://en.cppreference.com/w/cpp/container/span
 */
template <typename T>
class Span {
private:
    template <typename, typename = void>
    struct HasSize : std::false_type {};

    template <typename C>
    struct HasSize<C, std::void_t<decltype(std::size(std::declval<C>()))>> : std::true_type {};

    template <typename, typename = void>
    struct HasData : std::false_type {};

    template <typename C>
    struct HasData<C, std::void_t<decltype(std::data(std::declval<C>()))>> : std::true_type {};

    template <typename C>
    using EnableIfHasSizeAndData =
        typename std::enable_if_t<HasSize<C>::value && HasData<C>::value>;

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

    constexpr Span() noexcept = default;

    constexpr Span(T* data, size_t size) noexcept
        : size_(size),
          data_(data) {}

    /**
     * Implicit constructor from a container like `std::array` or `std::vector`.
     */
    template <typename Container, typename = EnableIfHasSizeAndData<Container>>
    constexpr Span(Container& container) noexcept  // NOLINT
        : Span(std::data(container), std::size(container)) {}

    /**
     * Implicit constructor from a container like `std::array` or `std::vector` (const).
     */
    template <typename Container, typename = EnableIfHasSizeAndData<Container>>
    constexpr Span(const Container& container) noexcept  // NOLINT
        : Span(std::data(container), std::size(container)) {}

    /**
     * Implicit constructor from an initializer list.
     *
     * Only safe to use if `std::initializer_list` itself outlives the Span:
     * @code{.cpp}
     * void takeView(Span<const int> values);
     * // ok
     * takeView({1, 2, 3});
     * // not ok
     * Span<const int> values = {1, 2, 3};
     * takeView(values);
     * @endcode
     */
    constexpr Span(std::initializer_list<value_type> values) noexcept  // NOLINT
        : Span(values.begin(), values.size()) {}

    /// Explicit conversion to `std::vector`.
    template <typename Allocator>
    explicit operator std::vector<value_type, Allocator>() const {
        return {cbegin(), cend()};
    }

    constexpr void swap(Span& other) noexcept {
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
        assert(index < size());
        return data()[index];
    }

    [[nodiscard]] constexpr reference front() const noexcept {
        assert(!empty());
        return *data();
    }

    [[nodiscard]] constexpr reference back() const noexcept {
        assert(!empty());
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

    /// Obtain a view over `count` elements of this Span starting at offset `offset`.
    [[nodiscard]] constexpr Span subview(
        size_t offset, size_t count = (std::numeric_limits<std::size_t>::max)()
    ) const noexcept {
        if (offset >= size()) {
            return {};
        }
        if (count > size() - offset) {
            count = size() - offset;
        }
        return {data() + offset, count};
    }

    /// Obtain a view over the first `count` elements of this Span.
    [[nodiscard]] constexpr Span first(size_t count) const noexcept {
        if (count >= size()) {
            return *this;
        }
        return {data(), count};
    }

    /// Obtain a view over the last `count` elements of this Span.
    [[nodiscard]] constexpr Span last(size_t count) const noexcept {
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
Span(Container&) -> Span<typename Container::value_type>;

template <typename Container>
Span(const Container&) -> Span<const typename Container::value_type>;

/* ------------------------------------------- Helper ------------------------------------------- */

namespace detail {

template <typename>
struct IsSpan : std::false_type {};

template <typename T>
struct IsSpan<Span<T>> : std::true_type {};

}  // namespace detail

/* ----------------------------------------- Comparison ----------------------------------------- */

namespace detail {

template <typename T, typename U>
using EnableIfEqualityComparable = typename std::enable_if_t<
    std::is_invocable_v<std::equal_to<>, const T&, const U&> &&
    std::is_invocable_v<std::not_equal_to<>, const T&, const U&>>;

}  // namespace detail

template <typename T, typename U, typename = detail::EnableIfEqualityComparable<T, U>>
constexpr bool operator==(Span<T> lhs, Span<U> rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T, typename U, typename = detail::EnableIfEqualityComparable<T, U>>
constexpr bool operator!=(Span<T> lhs, Span<U> rhs) {
    return !(lhs == rhs);
}

}  // namespace opcua
