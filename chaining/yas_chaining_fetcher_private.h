//
//  yas_chaining_fetcher_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"

namespace yas::chaining {
template <typename T>
struct fetcher<T>::impl : sender_base<T>::impl {
    std::function<opt_t<T>(void)> _fetching_handler;

    impl(std::function<opt_t<T>(void)> &&handler) : _fetching_handler(std::move(handler)) {
    }

    void sync(std::uintptr_t const key) override {
        if (auto value = this->_fetching_handler()) {
            if (auto joint = this->joints.at(key).lock()) {
                joint.call_first(*value);
            }
        }
    }

    void broadcast() {
        if (auto lock = std::unique_lock<std::mutex>(this->_sync_mutex, std::try_to_lock); lock.owns_lock()) {
            if (auto value = this->_fetching_handler()) {
                for (auto &pair : this->joints) {
                    if (auto joint = pair.second.lock()) {
                        joint.call_first(*value);
                    }
                }
            }
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
    std::mutex _sync_mutex;
    chaining::receiver<> _receiver{nullptr};
};

template <typename T>
fetcher<T>::fetcher(std::function<opt_t<T>(void)> handler)
    : sender_base<T>(std::make_shared<impl>(std::move(handler))) {
}

template <typename T>
fetcher<T>::fetcher(std::nullptr_t) : sender_base<T>(nullptr) {
}

template <typename T>
void fetcher<T>::broadcast() const {
    this->template impl_ptr<impl>()->broadcast();
}

template <typename T>
chaining::chain<T, T, T, true> fetcher<T>::chain() {
    return this->template impl_ptr<impl>()->template begin<true>(*this);
}

template <typename T>
chaining::receiver<> &fetcher<T>::receiver() {
    return this->template impl_ptr<impl>()->receiver();
}
}  // namespace yas::chaining
