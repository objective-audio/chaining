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
void chaining::perform_receiver<T>::receive_value(T const &value) {
    this->_impl->receive_value(value);
}

template <typename T>
std::shared_ptr<chaining::perform_receiver<T>> chaining::perform_receiver<T>::make_shared(
    std::function<void(T const &)> const &handler) {
    return std::shared_ptr<chaining::perform_receiver<T>>(new chaining::perform_receiver<T>{handler});
}

template <typename T>
std::shared_ptr<chaining::perform_receiver<T>> chaining::perform_receiver<T>::make_shared(
    std::function<void(T const &)> &&handler) {
    return std::shared_ptr<chaining::perform_receiver<T>>(new chaining::perform_receiver<T>{std::move(handler)});
}

template <typename T>
std::shared_ptr<chaining::perform_receiver<T>> chaining::perform_receiver<T>::make_shared(
    std::function<void(void)> const &handler) {
    return make_shared([handler](auto const &) { handler(); });
}

template <typename T>
std::shared_ptr<chaining::perform_receiver<T>> chaining::perform_receiver<T>::make_shared(
    std::function<void(void)> &&handler) {
    return make_shared([handler = std::move(handler)](auto const &) { handler(); });
}
}  // namespace yas::chaining
