//
//  yas_chaining_notifier_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"

namespace yas::chaining {
template <typename T>
struct notifier<T>::impl : sender<T>::impl {
    void locked_send_value(T const &value) {
        if (auto lock = std::unique_lock<std::mutex>(this->_send_mutex, std::try_to_lock); lock.owns_lock()) {
            this->broadcast(value);
        }
    }

    chaining::receiver<T> &receiver() {
        if (!this->_receiver) {
            this->_receiver = chaining::receiver<T>{
                [weak_notifier = to_weak(this->template cast<chaining::notifier<T>>())](T const &value) {
                    if (auto notifier = weak_notifier.lock()) {
                        notifier.notify(value);
                    }
                }};
        }
        return this->_receiver;
    }

   private:
    std::mutex _send_mutex;
    chaining::receiver<T> _receiver{nullptr};
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
chain<T, T, T, false> notifier<T>::chain() {
    return this->template impl_ptr<impl>()->template chain<false>();
}

template <typename T>
receiver<T> &notifier<T>::receiver() {
    return this->template impl_ptr<impl>()->receiver();
}
}  // namespace yas::chaining
