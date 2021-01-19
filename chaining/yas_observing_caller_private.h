//
//  yas_observing_caller_private.h
//

#pragma once

namespace yas::observing {
template <typename T>
caller<T>::~caller() {
    for (auto const &canceller : this->_cancellers) {
        if (auto shared = canceller.lock()) {
            shared->ignore();
        }
    }
}

template <typename T>
canceller_ptr caller<T>::add(handler_f &&handler) {
    this->_handlers.emplace(this->_next_idx, std::move(handler));
    auto canceller =
        canceller::make_shared(this->_next_idx, [this](uint32_t const idx) { this->_handlers.erase(idx); });
    this->_cancellers.emplace_back(canceller);
    ++this->_next_idx;
    return canceller;
}

template <typename T>
void caller<T>::call(T const &value) {
    for (auto const &pair : this->_handlers) {
        pair.second(value);
    }
}
}  // namespace yas::observing
