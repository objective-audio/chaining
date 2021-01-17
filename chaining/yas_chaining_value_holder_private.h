//
//  yas_chaining_holder_private.h
//

#pragma once

#include <chaining/yas_chaining_chain.h>

namespace yas::chaining::value {
template <typename T>
holder<T>::holder(T &&value) : _value(std::move(value)) {
}

template <typename T>
holder<T>::~holder() = default;

template <typename T>
void holder<T>::set_value(T &&value) {
    if (this->_value != value) {
        this->_value = std::move(value);
        this->broadcast(this->_value);
    }
}

template <typename T>
void holder<T>::set_value(T const &value) {
    T copied = value;
    this->set_value(std::move(copied));
}

template <typename T>
[[nodiscard]] T const &holder<T>::value() const {
    return this->_value;
}

template <typename T>
[[nodiscard]] T &holder<T>::value() {
    return this->_value;
}

template <typename T>
chain_sync_t<T> holder<T>::chain() {
    return this->chain_sync();
}

template <typename T>
void holder<T>::receive_value(T const &value) {
    this->set_value(value);
}

template <typename T>
void holder<T>::fetch_for(any_joint const &joint) const {
    this->send_value_to_target(this->value(), joint.identifier());
}

template <typename T>
holder_ptr<T> holder<T>::make_shared(T value) {
    return std::shared_ptr<holder<T>>(new holder<T>{std::move(value)});
}
}  // namespace yas::chaining::value
