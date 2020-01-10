//
//  yas_chaining_notifier_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"

namespace yas::chaining {
template <typename T>
notifier<T>::notifier() {
}

template <typename T>
void notifier<T>::notify(T const &value) {
    this->broadcast(value);
}

template <typename T>
chain_unsync_t<T> notifier<T>::chain() {
    return this->chain_unsync();
}

template <typename T>
void notifier<T>::receive_value(T const &value) {
    return this->notify(value);
}

template <typename T>
void notifier<T>::fetch_for(any_joint const &) const {
    // do nothing
}

template <typename T>
notifier_ptr<T> notifier<T>::make_shared() {
    return std::shared_ptr<notifier<T>>(new notifier<T>{});
}
}  // namespace yas::chaining
