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
    this->_handlers.emplace(this->_next_idx, handler_container{.handler = handler});
    auto canceller = canceller::make_shared(this->_next_idx,
                                            [this](uint32_t const idx) { this->_handlers.at(idx).enabled = false; });
    this->_cancellers.emplace_back(canceller);
    ++this->_next_idx;
    return canceller;
}

template <typename T>
void caller<T>::call(T const &value) {
    if (!this->_calling) {
        this->_calling = true;
        for (auto const &pair : this->_handlers) {
            if (pair.second.enabled) {
                pair.second.handler(value);
            }
        }
        this->_calling = false;
    }
}
}  // namespace yas::observing
