//
//  yas_chaining_holder_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"

namespace yas::chaining::value {
template <typename T>
struct holder<T>::impl : sender<T>::impl {};

template <typename T>
holder<T>::holder(T &&value) : sender<T>(std::make_shared<impl>()), _value(std::move(value)) {
}

template <typename T>
holder<T>::holder(std::shared_ptr<impl> &&impl) : sender<T>(std::move(impl)) {
}

template <typename T>
holder<T>::~holder() = default;

template <typename T>
void holder<T>::set_value(T &&value) {
    if (auto lock = std::unique_lock<std::mutex>(this->_set_mutex, std::try_to_lock); lock.owns_lock()) {
        if (this->_value != value) {
            this->_value = std::move(value);
            this->broadcast(this->_value);
        }
    }
}

template <typename T>
void holder<T>::set_value(T const &value) {
    T copied = value;
    this->set_value(std::move(copied));
}

template <typename T>
[[nodiscard]] T const &holder<T>::raw() const { return this->_value; }

template <typename T>
[[nodiscard]] T &holder<T>::raw() { return this->_value; }

template <typename T>
chain_sync_t<T> holder<T>::chain() {
    return this->chain_sync();
}

template <typename T>
void holder<T>::receive_value(T const &value) {
    this->set_value(value);
}

template <typename T>
bool holder<T>::is_equal(sender<T> const &rhs) const {
    auto sendable_ptr = rhs.shared_from_this();
    auto rhs_ptr = std::dynamic_pointer_cast<typename value::holder<T> const>(sendable_ptr);
    if (rhs_ptr) {
        return this->_value == rhs_ptr->_value;
    } else {
        return false;
    }
}

template <typename T>
void holder<T>::fetch_for(any_joint const &joint) {
    this->send_value_to_target(this->raw(), joint.identifier());
}

template <typename T>
std::shared_ptr<holder<T>> holder<T>::make_shared(T value) {
    return std::shared_ptr<holder<T>>(new holder<T>{std::move(value)});
}
}  // namespace yas::chaining::value
