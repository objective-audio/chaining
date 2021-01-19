//
//  yas_observing_notifier_private.h
//

#pragma once

namespace yas::observing {
template <typename T>
notifier<T>::notifier() {
}

template <typename T>
void notifier<T>::notify(T const &value) {
    this->_caller.call(value);
}

template <typename T>
canceller_ptr notifier<T>::observe(typename caller<T>::handler_f &&handler) {
    return this->_caller.add(std::move(handler));
}

template <typename T>
notifier_ptr<T> notifier<T>::make_shared() {
    return std::shared_ptr<notifier<T>>(new notifier<T>{});
}
}  // namespace yas::observing
