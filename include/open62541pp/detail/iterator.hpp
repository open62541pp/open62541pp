#pragma once

#include <functional>  // invoke
#include <iterator>
#include <type_traits>
#include <utility>  // move

namespace opcua::detail {

template <typename Iter, typename F>
class TransformIterator {
public:
    using iterator_type = Iter;
    using iterator_category = typename std::iterator_traits<Iter>::iterator_category;
    using value_type =
        typename std::invoke_result_t<F, typename std::iterator_traits<Iter>::value_type>;
    using difference_type = typename std::iterator_traits<Iter>::difference_type;
    using pointer = value_type*;
    using reference = value_type&;

    constexpr TransformIterator(Iter it, F func)
        : it_{std::move(it)},
          func_{std::move(func)} {}

    constexpr const Iter& base() const& noexcept {
        return it_;
    }

    constexpr Iter base() && {
        return it_;
    }

    constexpr value_type operator*() const {
        return std::invoke(func_, *it_);
    }

    constexpr value_type operator[](difference_type n) const {
        return *(*this + n);
    }

    constexpr TransformIterator& operator++() {
        ++it_;
        return *this;
    }

    constexpr TransformIterator operator++(int) {  // NOLINT(cert-dcl21-cpp)
        auto temp = *this;
        ++(*this);
        return temp;
    }

    constexpr TransformIterator operator+(difference_type n) const {
        return {it_ + n, func_};
    }

    constexpr TransformIterator& operator+=(difference_type n) {
        it_ += n;
        return *this;
    }

    constexpr TransformIterator& operator--() {
        --it_;
        return *this;
    }

    constexpr TransformIterator operator--(int) {  // NOLINT(cert-dcl21-cpp)
        auto temp = *this;
        --(*this);
        return temp;
    }

    constexpr TransformIterator operator-(difference_type n) const {
        return {it_ - n, func_};
    }

    constexpr TransformIterator& operator-=(difference_type n) {
        it_ -= n;
        return *this;
    }

private:
    Iter it_;
    std::decay_t<F> func_;
};

template <typename Iter1, typename F1, typename Iter2, typename F2>
constexpr auto operator==(
    const TransformIterator<Iter1, F1>& lhs, const TransformIterator<Iter2, F2>& rhs
) {
    return lhs.base() == rhs.base();
}

template <typename Iter1, typename F1, typename Iter2, typename F2>
constexpr auto operator!=(
    const TransformIterator<Iter1, F1>& lhs, const TransformIterator<Iter2, F2>& rhs
) {
    return lhs.base() != rhs.base();
}

template <typename Iter1, typename F1, typename Iter2, typename F2>
constexpr auto operator<(
    const TransformIterator<Iter1, F1>& lhs, const TransformIterator<Iter2, F2>& rhs
) {
    return lhs.base() < rhs.base();
}

template <typename Iter1, typename F1, typename Iter2, typename F2>
constexpr auto operator<=(
    const TransformIterator<Iter1, F1>& lhs, const TransformIterator<Iter2, F2>& rhs
) {
    return lhs.base() <= rhs.base();
}

template <typename Iter1, typename F1, typename Iter2, typename F2>
constexpr auto operator>(
    const TransformIterator<Iter1, F1>& lhs, const TransformIterator<Iter2, F2>& rhs
) {
    return lhs.base() > rhs.base();
}

template <typename Iter1, typename F1, typename Iter2, typename F2>
constexpr auto operator>=(
    const TransformIterator<Iter1, F1>& lhs, const TransformIterator<Iter2, F2>& rhs
) {
    return lhs.base() >= rhs.base();
}

template <typename Iter, typename F>
constexpr TransformIterator<Iter, F> operator+(
    const TransformIterator<Iter, F>& it, typename TransformIterator<Iter, F>::difference_type n
) {
    return {it.base() + n, it.func_};
}

template <typename Iter1, typename F1, typename Iter2, typename F2>
constexpr auto operator-(
    const TransformIterator<Iter1, F1>& lhs, const TransformIterator<Iter2, F2>& rhs
) {
    return lhs.base() - rhs.base();
}

}  // namespace opcua::detail
