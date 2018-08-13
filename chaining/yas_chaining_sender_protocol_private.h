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
void sender_chainable<T>::erase_joint(std::uintptr_t const key) {
    impl_ptr<impl>()->erase_joint(key);
}

template <typename T>
void sender_chainable<T>::sync(std::uintptr_t const key) {
    impl_ptr<impl>()->sync(key);
}
}  // namespace yas::chaining
