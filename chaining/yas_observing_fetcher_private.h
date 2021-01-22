//
//  yas_observing_fetcher_private.h
//

#pragma once

namespace yas::observing {
template <typename T>
fetcher<T>::fetcher(std::function<std::optional<T>(void)> &&handler) : _fetching_handler(std::move(handler)) {
}

template <typename T>
std::optional<T> fetcher<T>::fetched_value() const {
    return this->_fetching_handler();
}

template <typename T>
void fetcher<T>::push() {
    if (auto const fetched = this->fetched_value(); fetched.has_value()) {
        this->_caller.call(fetched.value());
    }
}

template <typename T>
void fetcher<T>::push(T const &value) {
    this->_caller.call(value);
}

template <typename T>
canceller_ptr fetcher<T>::observe(typename caller<T>::handler_f &&handler, bool const sync) {
    if (sync) {
        if (auto const fetched = this->fetched_value(); fetched.has_value()) {
            handler(fetched.value());
        }
    }
    return this->_caller.add(std::move(handler));
}

template <typename T>
fetcher_ptr<T> fetcher<T>::make_shared(std::function<std::optional<T>(void)> handler) {
    return fetcher_ptr<T>(new fetcher<T>{std::move(handler)});
}
}  // namespace yas::observing
