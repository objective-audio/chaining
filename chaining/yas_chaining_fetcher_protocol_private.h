//
//  yas_chaining_fetcher_protocol_private.h
//

#pragma once

namespace yas::chaining {
template <typename T>
fetchable<T>::fetchable(std::shared_ptr<impl> impl) : protocol(std::move(impl)) {
}

template <typename T>
fetchable<T>::fetchable(std::nullptr_t) : protocol(nullptr) {
}

template <typename T>
opt_t<T> fetchable<T>::fetched_value() {
    return this->template impl_ptr<impl>()->fetched_value();
}
}  // namespace yas::chaining
