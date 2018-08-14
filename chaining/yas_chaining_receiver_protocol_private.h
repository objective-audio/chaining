//
//  yas_chaining_receiver_chainable_private.h
//

#pragma once

namespace yas::chaining {
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
}  // namespace yas::chaining
