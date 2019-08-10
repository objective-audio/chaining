//
//  yas_chaining_holder_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"

namespace yas::chaining::value {
template <typename T>
struct holder<T>::impl : sender<T>::impl {
    impl(T &&value) : _value(std::move(value)) {
    }

    void fetch_for(any_joint const &joint) override {
        this->send_value_to_target(this->_value, joint.identifier());
    }

    T _value;
    std::mutex _set_mutex;
};

template <typename T>
holder<T>::holder(T &&value) : sender<T>(std::make_shared<impl>(std::move(value))) {
}

template <typename T>
holder<T>::holder(std::shared_ptr<impl> &&impl) : sender<T>(std::move(impl)) {
}

template <typename T>
holder<T>::~holder() = default;

template <typename T>
void holder<T>::set_value(T &&value) {
    auto impl_ptr = this->template impl_ptr<impl>();
    if (auto lock = std::unique_lock<std::mutex>(impl_ptr->_set_mutex, std::try_to_lock); lock.owns_lock()) {
        if (impl_ptr->_value != value) {
            impl_ptr->_value = std::move(value);
            impl_ptr->broadcast(impl_ptr->_value);
        }
    }
}

template <typename T>
void holder<T>::set_value(T const &value) {
    T copied = value;
    this->set_value(std::move(copied));
}

template <typename T>
[[nodiscard]] T const &holder<T>::raw() const { return this->template impl_ptr<impl>()->_value; }

template <typename T>
[[nodiscard]] T &holder<T>::raw() { return this->template impl_ptr<impl>()->_value; }

template <typename T>
chain_sync_t<T> holder<T>::chain() const {
    return this->template impl_ptr<impl>()->chain_sync();
}

template <typename T>
void holder<T>::receive_value(T const &value) {
    this->set_value(value);
}

template <typename T>
bool holder<T>::is_equal(sender<T> const &rhs) const {
    auto lhs_impl = this->template impl_ptr<impl>();
    auto rhs_impl = rhs.template impl_ptr<impl>();
    if (lhs_impl && rhs_impl) {
        return lhs_impl->_value == rhs_impl->_value;
    } else {
        return false;
    }
}

template <typename T>
std::shared_ptr<holder<T>> holder<T>::make_shared(T value) {
    return std::shared_ptr<holder<T>>(new holder<T>{std::move(value)});
}
}  // namespace yas::chaining::value
