//
//  yas_chaining_receiver_private.h
//

#pragma once

#include "yas_chaining_receiver_protocol.h"

namespace yas::chaining {
template <typename T>
struct chaining::receiver<T>::impl : base::impl, chaining::receiver_chainable<T>::impl {
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
chaining::receiver_chainable<T> chaining::receiver<T>::chainable() {
    if (!this->_chainable) {
        this->_chainable = chaining::receiver_chainable<T>{impl_ptr<typename chaining::receiver_chainable<T>::impl>()};
    }
    return this->_chainable;
}
}  // namespace yas::chaining
