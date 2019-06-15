//
//  yas_chaining_notifier_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"

namespace yas::chaining {
template <typename T>
struct notifier<T>::impl : sender<T>::impl, chaining::receivable<T>::impl {
    void locked_send_value(T const &value) {
        if (auto lock = std::unique_lock<std::mutex>(this->_send_mutex, std::try_to_lock); lock.owns_lock()) {
            this->broadcast(value);
        }
    }

    void receive_value(T const &value) {
        this->locked_send_value(value);
    }

   private:
    std::mutex _send_mutex;
};

template <typename T>
notifier<T>::notifier() : sender<T>(std::make_shared<impl>()) {
}

template <typename T>
notifier<T>::notifier(std::shared_ptr<impl> &&impl) : sender<T>(std::move(impl)) {
}

template <typename T>
notifier<T>::notifier(std::nullptr_t) : sender<T>(nullptr) {
}

template <typename T>
void notifier<T>::notify(T const &value) {
    this->template impl_ptr<impl>()->locked_send_value(value);
}

template <typename T>
chain_unsync_t<T> notifier<T>::chain() const {
    return this->template impl_ptr<impl>()->chain_unsync();
}

template <typename T>
chaining::receivable<T> notifier<T>::receivable() {
    if (!this->_receivable) {
        this->_receivable = chaining::receivable<T>{this->template impl_ptr<typename chaining::receivable<T>::impl>()};
    }
    return this->_receivable;
}
}  // namespace yas::chaining
