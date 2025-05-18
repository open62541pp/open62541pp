#pragma once

#include <algorithm>  // for_each, transform
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <memory>  // destroy, uninitialized_default_construct
#include <new>  // bad_alloc
#include <stdexcept>  // out_of_range
#include <type_traits>
#include <utility>  // exchange

#include "open62541pp/detail/open62541/common.h"  // UA_free, UA_malloc
#include "open62541pp/detail/traits.hpp"  // IsIterator
#include "open62541pp/wrapper.hpp"  // TypeHandler

namespace opcua {

template <typename T, typename Handler = TypeHandler<T>>
class Array {
    template <typename InputIt>
    using EnableIfIterator = typename std::enable_if_t<detail::IsIterator<InputIt>::value>;

public:
    static_assert(std::is_nothrow_move_constructible_v<T>);
    static_assert(std::is_invocable_v<decltype(Handler::copy), const T&>);
    static_assert(std::is_nothrow_invocable_v<decltype(Handler::move), T&&>);

    using HandlerType = Handler;

    // clang-format off
    using value_type             = T;
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

    struct Storage {
        size_t size;
        T* data;
    };

    struct Deleter {
        void operator()(T* data, size_t size) noexcept {
            if constexpr (detail::HasClear<Handler, T>::value) {
                std::for_each_n(data, size, Handler::clear);
            }
            std::destroy_n(data, size);
            UA_free(data);
        }

        void operator()(Storage& storage) noexcept {
            operator()(storage.data, storage.size);
            storage = {};
        }
    };

    Array() noexcept = default;

    /// Create array with given size.
    explicit Array(size_t size) {
        init(size);
    }

    /// Create array with `count` copies of `value`.
    Array(size_t count, const T& value)
        : Array(count) {
        std::fill(begin(), end(), value);
    }

    /// Create array from a range of elements (copy).
    template <typename InputIt, typename = EnableIfIterator<InputIt>>
    Array(InputIt first, InputIt last) {
        init(first, last);
    }

    /// Create array from initializer list (copy).
    Array(std::initializer_list<T> values)
        : Array(values.begin(), values.end()) {}

    /// Copy constructor.
    Array(const Array& other)
        : Array(other.begin(), other.end()) {}

    /// Move constructor.
    Array(Array&& other) noexcept
        : storage_{std::exchange(other.storage_, {})} {}

    ~Array() noexcept {
        clear();
    }

    /// Copy assignment operator.
    Array& operator=(const Array& other) {
        if (this != &other) {
            clear();
            init(other.begin(), other.end());
        }
        return *this;
    }

    /// Move assignment operator.
    Array& operator=(Array&& other) noexcept {
        if (this != &other) {
            clear();
            storage_ = std::exchange(other.storage_, {});
        }
        return *this;
    }

    /// Take ownership of array.
    [[nodiscard]] static Array adopt(T* data, size_t size) noexcept {
        return Array{data, size};
    }

    size_t size() const noexcept {
        return storage_.size;
    }

    bool empty() const noexcept {
        return size() == 0;
    }

    T* data() noexcept {
        return storage_.data;
    }

    const T* data() const noexcept {
        return storage_.data;
    }

    /// Access element by index.
    T& operator[](size_t index) noexcept {
        assert(index < size());
        return data()[index];
    }

    /// @copydoc operator[]
    const T& operator[](size_t index) const noexcept {
        assert(index < size());
        return data()[index];
    }

    /// Access element by index with bounds checking.
    /// @exception std::out_of_range If `index` >= size()
    T& at(size_t index) {
        if (index >= size()) {
            throw std::out_of_range("index >= size()");
        }
        return data()[index];
    }

    /// @copydoc at
    const T& at(size_t index) const {
        if (index >= size()) {
            throw std::out_of_range("index >= size()");
        }
        return data()[index];
    }

    T& front() noexcept {
        assert(!empty());
        return *data();
    }

    const T& front() const noexcept {
        assert(!empty());
        return *data();
    }

    T& back() noexcept {
        assert(!empty());
        return *(data() + size() - 1);
    }

    const T& back() const noexcept {
        assert(!empty());
        return *(data() + size() - 1);
    }

    iterator begin() noexcept {
        return {data()};
    }

    const_iterator begin() const noexcept {
        return {data()};
    }

    const_iterator cbegin() const noexcept {
        return {data()};
    }

    iterator end() noexcept {
        return {data() + size()};
    }

    const_iterator end() const noexcept {
        return {data() + size()};
    }

    const_iterator cend() const noexcept {
        return {data() + size()};
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator{end()};
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator{end()};
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator{cend()};
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator{begin()};
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator{begin()};
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator{cbegin()};
    }

    /// Erase all elements from array.
    void clear() noexcept {
        Deleter{}(storage_);
    }

    /// Resize array.
    void resize(size_t newSize) {
        if (newSize == size()) {
            return;
        }
        Array tmp(newSize);
        std::transform(mbegin(), mbegin() + std::min(size(), newSize), tmp.begin(), Handler::move);
        swap(tmp);
    }

    /// Replace contents with `count` copies of `value`.
    void assign(size_t count, const T& value) {
        Array tmp(count, value);
        swap(tmp);
    }

    /// Replace contents with elements from range [`first`, `last`).
    template <typename InputIt, typename = EnableIfIterator<InputIt>>
    void assign(InputIt first, InputIt last) {
        Array tmp(first, last);
        swap(tmp);
    }

    /// Replace contents with elements from initializer list `values`.
    void assign(std::initializer_list<T> values) {
        Array tmp(values);
        swap(tmp);
    }

    /// Insert `value` before `pos` (copy).
    iterator insert(const_iterator pos, const T& value) {
        return insertImpl(pos, value);
    }

    /// Insert `value` before `pos` (move).
    iterator insert(const_iterator pos, T&& value) {
        return insertImpl(pos, std::move(value));
    }

    /// Insert `count` copies of `value` before `pos`.
    iterator insert(const_iterator pos, size_t count, const T& value) {
        return insertImpl(pos, count, value);
    }

    /// Insert elements from range [`first`, `last`) before `pos`.
    template <typename InputIt, typename = EnableIfIterator<InputIt>>
    iterator insert(const_iterator pos, InputIt first, InputIt last) {
        return insertImpl(pos, first, last);
    }

    /// Insert elements from initializer list `values` before `pos`.
    iterator insert(const_iterator pos, std::initializer_list<T> values) {
        return insertImpl(pos, values.begin(), values.end());
    }

    /// Remove element at `pos`.
    iterator erase(const_iterator pos) {
        return eraseImpl(pos);
    }

    /// Remove elements in the range [`first`, `last`).
    iterator erase(const_iterator first, const_iterator last) {
        return eraseImpl(first, last);
    }

    /// Swap arrays.
    void swap(Array& other) noexcept {
        std::swap(storage_, other.storage_);
    }

    /// Release the ownership of the array.
    /// @note The returned array must be deleted manually with @ref Deleter.
    [[nodiscard]] Storage release() noexcept {
        return std::exchange(storage_, {});
    }

private:
    Array(T* data, size_t size) noexcept
        : storage_{size, data} {}

    [[nodiscard]] static T* allocate(size_t size) {
        if (size == 0) {
            return nullptr;
        }
        if (size > UA_INT32_MAX) {
            throw std::bad_alloc{};
        }
        T* ptr = static_cast<T*>(UA_calloc(size, sizeof(T)));  // NOLINT
        if (ptr == nullptr) {
            throw std::bad_alloc{};
        }
        std::uninitialized_default_construct_n(ptr, size);  // default-construct elements
        return ptr;
    }

    void init(size_t size) {
        storage_.size = size;
        storage_.data = allocate(size);
    }

    template <typename InputIt>
    void init(InputIt first, InputIt last) {
        init(first, last, typename std::iterator_traits<InputIt>::iterator_category{});
    }

    template <typename InputIt, typename Tag>
    void init(InputIt first, InputIt last, Tag /* unused */) {
        init(std::distance(first, last));
        std::transform(first, last, begin(), Handler::copy);
    }

    template <typename InputIt>
    void init(InputIt first, InputIt last, std::input_iterator_tag /* unused */) {
        size_t index = 0;
        size_t capacity = 16;  // initial capacity to avoid frequency reallocations
        Array tmp(capacity);
        for (auto it = first; it != last; ++it, ++index) {
            if (index >= capacity) {
                capacity *= 2;
                tmp.resize(capacity);
            }
            tmp[index] = Handler::copy(*it);
        }
        tmp.resize(index);
        swap(tmp);
    }

    size_t indexOf(const_iterator pos) const noexcept {
        assert(pos >= cbegin());
        assert(pos <= cend());
        return static_cast<size_t>(pos - cbegin());
    }

    std::move_iterator<iterator> mbegin() noexcept {
        return std::make_move_iterator(begin());
    }

    std::move_iterator<iterator> mend() noexcept {
        return std::make_move_iterator(end());
    }

    template <typename U>
    iterator insertImpl(const_iterator pos, U&& value) {
        const auto index = indexOf(pos);
        Array tmp(size() + 1);
        if constexpr (std::is_rvalue_reference_v<decltype(value)>) {
            tmp[index] = Handler::move(std::forward<U>(value));
        } else {
            tmp[index] = Handler::copy(value);
        }
        std::transform(mbegin(), mbegin() + index, tmp.begin(), Handler::move);
        std::transform(mbegin() + index, mend(), tmp.begin() + index + 1, Handler::move);
        swap(tmp);
        return begin() + index;
    }

    iterator insertImpl(const_iterator pos, size_t count, const T& value) {
        const auto index = indexOf(pos);
        Array tmp(size() + count);
        std::fill_n(tmp.begin() + index, count, value);
        std::transform(mbegin(), mbegin() + index, tmp.begin(), Handler::move);
        std::transform(mbegin() + index, mend(), tmp.begin() + index + count, Handler::move);
        swap(tmp);
        return begin() + index;
    }

    template <typename InputIt, typename = EnableIfIterator<InputIt>>
    iterator insertImpl(const_iterator pos, InputIt first, InputIt last) {
        return insertImpl(
            pos, first, last, typename std::iterator_traits<InputIt>::iterator_category{}
        );
    }

    template <typename InputIt, typename Tag, typename = EnableIfIterator<InputIt>>
    iterator insertImpl(const_iterator pos, InputIt first, InputIt last, Tag /* unused */) {
        using InputItRef = typename std::iterator_traits<InputIt>::reference;
        const auto index = indexOf(pos);
        const auto count = std::distance(first, last);
        if (count > 0) {
            Array tmp(size() + count);
            if constexpr (std::is_rvalue_reference_v<InputItRef>) {
                std::transform(first, last, tmp.begin() + index, Handler::move);
            } else {
                std::transform(first, last, tmp.begin() + index, Handler::copy);
            }
            std::transform(mbegin(), mbegin() + index, tmp.begin(), Handler::move);
            std::transform(mbegin() + index, mend(), tmp.begin() + index + count, Handler::move);
            swap(tmp);
        }
        return begin() + index;
    }

    template <typename InputIt, typename = EnableIfIterator<InputIt>>
    iterator insertImpl(
        const_iterator pos, InputIt first, InputIt last, std::input_iterator_tag /* unused */
    ) {
        Array tmp(first, last);
        return insertImpl(pos, tmp.mbegin(), tmp.mend());
    }

    iterator eraseImpl(const_iterator pos) {
        return eraseImpl(pos, pos + 1);
    }

    iterator eraseImpl(const_iterator first, const_iterator last) {
        const auto index = indexOf(first);
        const auto count = last - first;
        if (count > 0) {
            Array<T> tmp(size() - count);
            std::transform(mbegin(), mbegin() + index, tmp.begin(), Handler::move);
            std::transform(mbegin() + index + count, mend(), tmp.begin() + index, Handler::move);
            swap(tmp);
        }
        return begin() + index;
    }

    Storage storage_{};
};

/* ----------------------------------------- Comparison ----------------------------------------- */

/// @relates Array
template <typename T, typename Handler>
bool operator==(Array<T, Handler> lhs, Array<T, Handler> rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

/// @relates Array
template <typename T, typename Handler>
bool operator!=(Array<T, Handler> lhs, Array<T, Handler> rhs) {
    return !(lhs == rhs);
}

}  // namespace opcua
