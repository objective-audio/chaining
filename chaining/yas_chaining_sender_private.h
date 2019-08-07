//
//  yas_chaining_sender_private.h
//

#pragma once

#include <cpp_utils/yas_stl_utils.h>
#include <vector>
#include "yas_chaining_sender_protocol.h"

namespace yas::chaining {
template <typename T>
struct sender<T>::impl : chaining::sendable<T> {
    void fetch_for(any_joint const &joint) override {
    }

    uintptr_t identifier() const {
        return reinterpret_cast<uintptr_t>(this);
    }
};

template <typename T>
sender<T>::sender(std::shared_ptr<impl> &&impl) : _impl(std::move(impl)) {
}

template <typename T>
std::shared_ptr<sendable<T>> sender<T>::sendable() {
    return this->_impl;
}

template <typename T>
bool sender<T>::operator==(sender const &rhs) const {
    return _impl && rhs._impl && (_impl == rhs._impl || this->is_equal(rhs));
}

template <typename T>
bool sender<T>::operator!=(sender const &rhs) const {
    return !_impl || !rhs._impl || (_impl != rhs._impl && !this->is_equal(rhs));
}

template <typename T>
bool sender<T>::operator==(std::nullptr_t) const {
    return _impl == nullptr;
}

template <typename T>
bool sender<T>::operator!=(std::nullptr_t) const {
    return _impl != nullptr;
}

template <typename T>
template <typename Impl>
std::shared_ptr<Impl> sender<T>::impl_ptr() const {
    return std::dynamic_pointer_cast<Impl>(_impl);
}

template <typename T>
bool sender<T>::is_equal(sender<T> const &rhs) const {
    return this->_impl->identifier() == rhs._impl->identifier();
}
}  // namespace yas::chaining
