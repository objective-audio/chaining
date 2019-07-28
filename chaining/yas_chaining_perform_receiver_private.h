//
//  yas_chaining_receiver_private.h
//

#pragma once

namespace yas::chaining {
template <typename T>
struct chaining::perform_receiver<T>::impl : chaining::receivable<T> {
    std::function<void(T const &)> handler;

    impl(std::function<void(T const &)> const &handler) : handler(handler) {
    }

    impl(std::function<void(T const &)> &&handler) : handler(std::move(handler)) {
    }

    void receive_value(T const &value) override {
        this->handler(value);
    }
};

template <typename T>
chaining::perform_receiver<T>::perform_receiver(std::function<void(T const &)> const &handler)
    : _impl(std::make_shared<impl>(handler)) {
}

template <typename T>
chaining::perform_receiver<T>::perform_receiver(std::function<void(T const &)> &&handler)
    : _impl(std::make_shared<impl>(std::move(handler))) {
}

template <typename T>
chaining::perform_receiver<T>::perform_receiver(std::function<void(void)> const &handler)
    : perform_receiver([handler](auto const &) { handler(); }) {
}

template <typename T>
chaining::perform_receiver<T>::perform_receiver(std::function<void(void)> &&handler)
    : perform_receiver([handler = std::move(handler)](auto const &) { handler(); }) {
}

template <typename T>
chaining::receivable_ptr<T> chaining::perform_receiver<T>::receivable() {
    return this->_impl;
}
}  // namespace yas::chaining
