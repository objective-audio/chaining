//
//  yas_chaining_holder_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"

namespace yas::chaining {
template <typename T>
struct holder<T>::impl : sender<T>::impl {
    impl(T &&value) : _value(std::move(value)) {
    }

    T &value() {
        return this->_value;
    }

    void locked_set_value(T &&value) {
        if (auto lock = std::unique_lock<std::mutex>(this->_set_mutex, std::try_to_lock); lock.owns_lock()) {
            if (this->_value != value) {
                this->_value = std::move(value);
                this->broadcast(this->_value);
            }
        }
    }

    void fetch_for(any_joint const &joint) override {
        this->send_value_to_target(this->_value, joint.identifier());
    }

    chaining::receiver<T> &receiver() {
        if (!this->_receiver) {
            this->_receiver = chaining::receiver<T>{
                [weak_holder = to_weak(this->template cast<chaining::holder<T>>())](T const &value) {
                    if (auto holder = weak_holder.lock()) {
                        holder.set_value(value);
                    }
                }};
        }
        return this->_receiver;
    }

    virtual bool is_equal(std::shared_ptr<base::impl> const &rhs) const override {
        if (auto rhs_impl = std::dynamic_pointer_cast<typename holder<T>::impl>(rhs)) {
            return this->_value == rhs_impl->_value;
        } else {
            return false;
        }
    }

   private:
    T _value;
    std::mutex _set_mutex;
    chaining::receiver<T> _receiver{nullptr};
};

template <typename T>
holder<T>::holder(T value) : sender<T>(std::make_shared<impl>(std::move(value))) {
}

template <typename T>
holder<T>::holder(std::nullptr_t) : sender<T>(nullptr) {
}

template <typename T>
holder<T>::~holder() = default;

template <typename T>
T const &holder<T>::value() const {
    return this->template impl_ptr<impl>()->value();
}

template <typename T>
T &holder<T>::value() {
    return this->template impl_ptr<impl>()->value();
}

template <typename T>
void holder<T>::set_value(T value) {
    this->template impl_ptr<impl>()->locked_set_value(std::move(value));
}

template <typename T>
[[nodiscard]] T const &holder<T>::raw() const { return this->template impl_ptr<impl>()->value(); }

template <typename T>
chain_sync_t<T> holder<T>::chain() {
    return this->template impl_ptr<impl>()->chain_sync();
}

template <typename T>
receiver<T> &holder<T>::receiver() {
    return this->template impl_ptr<impl>()->receiver();
}
}  // namespace yas::chaining
