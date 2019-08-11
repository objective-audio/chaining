//
//  yas_chaining_fetcher_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"

namespace yas::chaining {
template <typename T>
struct fetcher<T>::impl : sender<T>::impl {
    impl(std::function<std::optional<T>(void)> &&handler) : _fetching_handler(std::move(handler)) {
    }

    std::function<std::optional<T>(void)> _fetching_handler;
};

template <typename T>
fetcher<T>::fetcher(std::function<std::optional<T>(void)> &&handler)
    : sender<T>(std::make_shared<impl>(std::move(handler))) {
}

template <typename T>
fetcher<T>::fetcher(std::shared_ptr<impl> &&impl) : sender<T>(std::move(impl)) {
}

template <typename T>
std::optional<T> fetcher<T>::fetched_value() const {
    return this->template impl_ptr<impl>()->_fetching_handler();
}

template <typename T>
void fetcher<T>::broadcast() {
    if (auto value = this->fetched_value()) {
        this->broadcast(*value);
    }
}

template <typename T>
void fetcher<T>::broadcast(T const &value) {
    this->sendable<T>::broadcast(value);
}

template <typename T>
chain_sync_t<T> fetcher<T>::chain() {
    return this->chain_sync();
}

template <typename T>
void fetcher<T>::receive_value(std::nullptr_t const &) {
    return this->broadcast();
}

template <typename T>
void fetcher<T>::fetch_for(any_joint const &joint) {
    if (auto value = this->fetched_value()) {
        this->send_value_to_target(*value, joint.identifier());
    }
}

template <typename T>
std::shared_ptr<fetcher<T>> fetcher<T>::make_shared(std::function<std::optional<T>(void)> handler) {
    return std::shared_ptr<fetcher<T>>(new fetcher<T>{std::move(handler)});
}
}  // namespace yas::chaining
