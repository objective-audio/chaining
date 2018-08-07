//
//  yas_chaining_utils.h
//

#pragma once

#include <algorithm>
#include <functional>

namespace yas::chaining {
template <typename T, typename U, std::enable_if_t<!std::is_same<T, U>::value, std::nullptr_t> = nullptr>
std::function<T(T const &)> add(U const &adding) {
    return [adding](T const &value) { return value + adding; };
}

template <typename T>
std::function<T(T const &)> add(T const &adding) {
    return [adding](T const &value) { return value + adding; };
}

template <typename T>
std::function<T(std::pair<T, T> const &)> add() {
    return [](std::pair<T, T> const &pair) { return pair.first + pair.second; };
}

template <typename T, typename U, std::enable_if_t<!std::is_same<T, U>::value, std::nullptr_t> = nullptr>
std::function<T(T const &)> multiply(U const &multiplying) {
    return [multiplying](T const &value) { return value * multiplying; };
}

template <typename T>
std::function<T(T const &)> multiply(T const &multiplying) {
    return [multiplying](T const &value) { return value * multiplying; };
}

template <typename T>
std::function<T(std::pair<T, T> const &)> multiply() {
    return [](std::pair<T, T> const &pair) { return pair.first * pair.second; };
}

template <typename T>
std::function<T(std::pair<T, T> const &)> min() {
    return [](std::pair<T, T> const &pair) { return std::min(pair.first, pair.second); };
}

template <typename T>
std::function<T(T const &)> min(T const &min) {
    return [min](T const &value) { return std::min(value, min); };
}

template <typename T>
std::function<T(std::pair<T, T> const &)> max() {
    return [](std::pair<T, T> const &pair) { return std::max(pair.first, pair.second); };
}

template <typename T>
std::function<T(T const &)> max(T const &max) {
    return [max](T const &value) { return std::max(value, max); };
}
}  // namespace yas::chaining
