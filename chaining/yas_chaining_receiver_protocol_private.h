//
//  yas_chaining_receiver_protocol_private.h
//

#pragma once

namespace yas::chaining {
template <typename T>
receivable<T>::receivable(std::shared_ptr<impl> impl) : protocol(std::move(impl)) {
}

template <typename T>
receivable<T>::receivable(std::nullptr_t) : protocol(nullptr) {
}

template <typename T>
void receivable<T>::receive_value(T const &value) {
    impl_ptr<impl>()->receive_value(value);
}
}  // namespace yas::chaining
