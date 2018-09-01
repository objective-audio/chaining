//
//  yas_chaining_fetcher_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"
#include "yas_chaining_fetcher_protocol_private.h"

namespace yas::chaining {
template <typename T>
struct fetcher<T>::impl : sender<T>::impl, chaining::fetchable<T>::impl {
    impl(std::function<opt_t<T>(void)> &&handler) : _fetching_handler(std::move(handler)) {
    }

    void fetch_for(any_joint const &joint) override {
        if (auto value = this->_fetching_handler()) {
            this->send_value_to_target(*value, joint.identifier());
        }
    }

    opt_t<T> fetched_value() override {
        return this->_fetching_handler();
    }

    void _broadcast() {
        if (auto value = this->_fetching_handler()) {
            this->broadcast(*value);
        }
    }

    chaining::receiver<> &receiver() {
        if (!this->_receiver) {
            this->_receiver =
                chaining::receiver<>{[weak_fetcher = to_weak(this->template cast<chaining::fetcher<T>>())] {
                    if (auto fetcher = weak_fetcher.lock()) {
                        fetcher.broadcast();
                    }
                }};
        }
        return this->_receiver;
    }

   private:
    std::function<opt_t<T>(void)> _fetching_handler;
    chaining::receiver<> _receiver{nullptr};
};

template <typename T>
fetcher<T>::fetcher(std::function<opt_t<T>(void)> handler) : sender<T>(std::make_shared<impl>(std::move(handler))) {
}

template <typename T>
fetcher<T>::fetcher(std::nullptr_t) : sender<T>(nullptr) {
}

template <typename T>
opt_t<T> fetcher<T>::fetched_value() const {
    return this->template impl_ptr<impl>()->fetched_value();
}

template <typename T>
void fetcher<T>::broadcast() const {
    this->template impl_ptr<impl>()->_broadcast();
}

template <typename T>
chaining::chain<T, T, T, true> fetcher<T>::chain() {
    return this->template impl_ptr<impl>()->template chain<true>(*this);
}

template <typename T>
chaining::receiver<> &fetcher<T>::receiver() {
    return this->template impl_ptr<impl>()->receiver();
}

template <typename T>
fetchable<T> fetcher<T>::fetchable() {
    if (!this->_fetchable) {
        this->_fetchable = chaining::fetchable<T>{this->template impl_ptr<typename chaining::fetchable<T>::impl>()};
    }
    return this->_fetchable;
}
}  // namespace yas::chaining
