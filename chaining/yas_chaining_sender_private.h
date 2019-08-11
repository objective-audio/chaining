//
//  yas_chaining_sender_private.h
//

#pragma once

#include <cpp_utils/yas_stl_utils.h>
#include <vector>
#include "yas_chaining_sender_protocol.h"

namespace yas::chaining {
template <typename T>
std::shared_ptr<sendable<T>> sender<T>::sendable() {
    return this->shared_from_this();
}

template <typename T>
bool sender<T>::operator==(sender const &rhs) const {
    return this->is_equal(rhs);
}

template <typename T>
bool sender<T>::operator!=(sender const &rhs) const {
    return !this->is_equal(rhs);
}

template <typename T>
uintptr_t sender<T>::identifier() const {
    auto shared = this->shared_from_this();
    return reinterpret_cast<uintptr_t>(shared.get());
}

template <typename T>
bool sender<T>::is_equal(sender<T> const &rhs) const {
    return this->identifier() == rhs.identifier();
}

template <typename T>
void sender<T>::fetch_for(any_joint const &joint) {
}
}  // namespace yas::chaining
