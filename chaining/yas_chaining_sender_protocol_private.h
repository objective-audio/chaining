//
//  yas_chaining_sender_protocol_private.h
//

#pragma once

namespace yas::chaining {
template <typename T>
sender_chainable<T>::sender_chainable(std::shared_ptr<impl> impl) : protocol(std::move(impl)) {
}

template <typename T>
sender_chainable<T>::sender_chainable(std::nullptr_t) : protocol(nullptr) {
}

template <typename T>
void sender_chainable<T>::broadcast(T const &value) {
    impl_ptr<impl>()->broadcast(value);
}

template <typename T>
void sender_chainable<T>::send_value_to_target(T const &value, std::uintptr_t const key) {
    impl_ptr<impl>()->send_value_to_target(value, key);
}

template <typename T>
void sender_chainable<T>::erase_joint(std::uintptr_t const key) {
    impl_ptr<impl>()->erase_joint(key);
}

template <typename T>
void sender_chainable<T>::fetch_for(any_joint const &joint) {
    impl_ptr<impl>()->fetch_for(joint);
}
}  // namespace yas::chaining
