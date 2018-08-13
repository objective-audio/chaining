//
//  yas_chaining_receiver_chainable_private.h
//

#pragma once

namespace yas::chaining {
template <typename T>
struct output<T>::impl : base::impl {
    weak<receiver<T>> _weak_receiver;

    impl(weak<receiver<T>> &&weak_receiver) : _weak_receiver(std::move(weak_receiver)) {
    }

    void output_value(T const &value) {
        if (auto receiver = this->_weak_receiver.lock()) {
            receiver.chainable().receive_value(value);
        }
    }
};

template <typename T>
output<T>::output(weak<receiver<T>> weak_receiver) : base(std::make_shared<impl>(std::move(weak_receiver))) {
}

template <typename T>
output<T>::output(std::nullptr_t) : base(nullptr) {
}

template <typename T>
void output<T>::output_value(T const &value) {
    impl_ptr<impl>()->output_value(value);
}

template <typename T>
receiver_chainable<T>::receiver_chainable(std::shared_ptr<impl> impl) : protocol(std::move(impl)) {
}

template <typename T>
receiver_chainable<T>::receiver_chainable(std::nullptr_t) : protocol(nullptr) {
}

template <typename T>
void receiver_chainable<T>::receive_value(T const &value) {
    impl_ptr<impl>()->receive_value(value);
}

template <typename T>
output<T> receiver_chainable<T>::make_output() {
    return impl_ptr<impl>()->make_output();
}
}  // namespace yas::chaining
