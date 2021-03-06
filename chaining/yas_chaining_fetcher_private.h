//
//  yas_chaining_fetcher_private.h
//

#pragma once

#include <chaining/yas_chaining_chain.h>

namespace yas::chaining {
template <typename T>
fetcher<T>::fetcher(std::function<std::optional<T>(void)> &&handler) : _fetching_handler(std::move(handler)) {
}

template <typename T>
std::optional<T> fetcher<T>::fetched_value() const {
    return this->_fetching_handler();
}

template <typename T>
void fetcher<T>::push() {
    if (auto value = this->fetched_value()) {
        this->push(*value);
    }
}

template <typename T>
void fetcher<T>::push(T const &value) {
    this->broadcast(value);
}

template <typename T>
chain_sync_t<T> fetcher<T>::chain() {
    return this->chain_sync();
}

template <typename T>
void fetcher<T>::receive_value(std::nullptr_t const &) {
    return this->push();
}

template <typename T>
void fetcher<T>::fetch_for(any_joint const &joint) const {
    if (auto value = this->fetched_value()) {
        this->send_value_to_target(*value, joint.identifier());
    }
}

template <typename T>
fetcher_ptr<T> fetcher<T>::make_shared(std::function<std::optional<T>(void)> handler) {
    return fetcher_ptr<T>(new fetcher<T>{std::move(handler)});
}
}  // namespace yas::chaining
