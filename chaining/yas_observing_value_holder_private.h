//
//  yas_observing_value_holder_private.h
//

#pragma once

namespace yas::observing::value {
template <typename T>
holder<T>::holder(T &&value) : _value(std::move(value)) {
}

template <typename T>
void holder<T>::set_value(T &&value) {
    if (this->_value != value) {
        this->_value = std::move(value);
        this->_caller.call(this->_value);
    }
}

template <typename T>
void holder<T>::set_value(T const &value) {
    if (this->_value != value) {
        T copied = value;
        this->set_value(std::move(copied));
    }
}

template <typename T>
[[nodiscard]] T const &holder<T>::value() const {
    return this->_value;
}

template <typename T>
[[nodiscard]] canceller_ptr holder<T>::observe(typename caller<T>::handler_f &&handler, bool const bind) {
    if (bind) {
        handler(this->_value);
    }
    return this->_caller.add(std::move(handler));
}

template <typename T>
[[nodiscard]] holder_ptr<T> holder<T>::make_shared(T const &value) {
    T copied = value;
    return make_shared(std::move(copied));
}

template <typename T>
[[nodiscard]] holder_ptr<T> holder<T>::make_shared(T &&value) {
    return std::shared_ptr<holder<T>>(new holder<T>{std::move(value)});
}
}  // namespace yas::observing::value
