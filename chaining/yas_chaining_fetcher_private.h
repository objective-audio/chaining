//
//  yas_chaining_fetcher_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"

namespace yas::chaining {
template <typename T>
struct fetcher<T>::impl : sender<T>::impl, weakable_impl {
    impl(std::function<std::optional<T>(void)> &&handler) : _fetching_handler(std::move(handler)) {
    }

    void fetch_for(any_joint const &joint) override {
        if (auto value = this->_fetching_handler()) {
            this->send_value_to_target(*value, joint.identifier());
        }
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
void fetcher<T>::broadcast() const {
    if (auto value = this->fetched_value()) {
        this->template impl_ptr<impl>()->broadcast(*value);
    }
}

template <typename T>
void fetcher<T>::broadcast(T const &value) const {
    this->template impl_ptr<impl>()->broadcast(value);
}

template <typename T>
chain_sync_t<T> fetcher<T>::chain() const {
    return this->template impl_ptr<impl>()->chain_sync();
}

template <typename T>
void fetcher<T>::receive_value(std::nullptr_t const &) {
    return this->broadcast();
}

template <typename T>
std::shared_ptr<weakable_impl> fetcher<T>::weakable_impl_ptr() const {
    return this->template impl_ptr<impl>();
}

template <typename T>
std::shared_ptr<fetcher<T>> fetcher<T>::make_shared(std::function<std::optional<T>(void)> handler) {
    return std::shared_ptr<fetcher<T>>(new fetcher<T>{std::move(handler)});
}
}  // namespace yas::chaining
