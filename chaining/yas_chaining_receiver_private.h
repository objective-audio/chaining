//
//  yas_chaining_receiver_private.h
//

#pragma once

#include "yas_chaining_receiver_protocol.h"

namespace yas::chaining {
template <typename T>
struct chaining::receiver<T>::impl : base::impl, chaining::receivable<T>::impl {
    std::function<void(T const &)> handler;

    impl(std::function<void(T const &)> &&handler) : handler(std::move(handler)) {
    }

    void receive_value(T const &value) override {
        this->handler(value);
    }
};

template <typename T>
chaining::receiver<T>::receiver(std::function<void(T const &)> handler)
    : base(std::make_shared<impl>(std::move(handler))) {
}

template <typename T>
chaining::receiver<T>::receiver(std::function<void(void)> handler)
    : receiver([handler = std::move(handler)](auto const &) { handler(); }) {
}

template <typename T>
chaining::receiver<T>::receiver(std::nullptr_t) : base(nullptr) {
}

template <typename T>
chaining::receiver<T>::~receiver() = default;

template <typename T>
chaining::receivable<T> chaining::receiver<T>::receivable() {
    if (!this->_receivable) {
        this->_receivable = chaining::receivable<T>{impl_ptr<typename chaining::receivable<T>::impl>()};
    }
    return this->_receivable;
}
}  // namespace yas::chaining
