//
//  yas_chaining_notifier_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"

namespace yas::chaining {
template <typename T>
struct notifier<T>::impl : sender<T>::impl {};

template <typename T>
notifier<T>::notifier() : sender<T>(std::make_shared<impl>()) {
}

template <typename T>
notifier<T>::notifier(std::shared_ptr<impl> &&impl) : sender<T>(std::move(impl)) {
}

template <typename T>
void notifier<T>::notify(T const &value) {
    if (auto lock = std::unique_lock<std::mutex>(this->_send_mutex, std::try_to_lock); lock.owns_lock()) {
        this->broadcast(value);
    }
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
std::shared_ptr<notifier<T>> notifier<T>::make_shared() {
    return std::shared_ptr<notifier<T>>(new notifier<T>{});
}
}  // namespace yas::chaining
