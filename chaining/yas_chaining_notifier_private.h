//
//  yas_chaining_notifier_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"

namespace yas::chaining {
template <typename T>
struct notifier<T>::impl : sender<T>::impl, chaining::receivable<T>, weakable_impl {
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
void notifier<T>::notify(T const &value) {
    this->template impl_ptr<impl>()->locked_send_value(value);
}

template <typename T>
chain_unsync_t<T> notifier<T>::chain() const {
    return this->template impl_ptr<impl>()->chain_unsync();
}

template <typename T>
chaining::receivable_ptr<T> notifier<T>::receivable() {
    return this->template impl_ptr<typename chaining::receivable<T>>();
}

template <typename T>
std::shared_ptr<weakable_impl> notifier<T>::weakable_impl_ptr() const {
    return this->template impl_ptr<impl>();
}

template <typename T>
std::shared_ptr<notifier<T>> notifier<T>::make_shared() {
    return std::shared_ptr<notifier<T>>(new notifier<T>{});
}
}  // namespace yas::chaining
