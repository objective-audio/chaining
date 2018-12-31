//
//  yas_chaining_sender_protocol_private.h
//

#pragma once

namespace yas::chaining {
template <typename T>
sendable<T>::sendable(std::shared_ptr<impl> impl) : protocol(std::move(impl)) {
}

template <typename T>
sendable<T>::sendable(std::nullptr_t) : protocol(nullptr) {
}

template <typename T>
void sendable<T>::broadcast(T const &value) {
    impl_ptr<impl>()->broadcast(value);
}

template <typename T>
void sendable<T>::send_value_to_target(T const &value, std::uintptr_t const key) {
    impl_ptr<impl>()->send_value_to_target(value, key);
}

template <typename T>
void sendable<T>::erase_joint(std::uintptr_t const key) {
    impl_ptr<impl>()->erase_joint(key);
}

template <typename T>
void sendable<T>::fetch_for(any_joint const &joint) {
    impl_ptr<impl>()->fetch_for(joint);
}

template <typename T>
chain_unsync_t<T> sendable<T>::chain_unsync() {
    return impl_ptr<impl>()->chain_unsync();
}

template <typename T>
chain_sync_t<T> sendable<T>::chain_sync() {
    return this->template impl_ptr<impl>()->chain_sync();
}
}  // namespace yas::chaining
