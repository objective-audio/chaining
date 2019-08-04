//
//  yas_chaining_holder_private.h
//

#pragma once

#include <mutex>
#include "yas_chaining_chain.h"

namespace yas::chaining::value {
template <typename T>
struct holder<T>::impl : sender<T>::impl, chaining::receivable<T>, weakable_impl {
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

    virtual bool is_equal(std::shared_ptr<typename sender<T>::impl> const &rhs) const override {
        if (auto rhs_impl = std::dynamic_pointer_cast<typename value::holder<T>::impl>(rhs)) {
            return this->_value == rhs_impl->_value;
        } else {
            return false;
        }
    }

    void receive_value(T const &value) override {
        T copied = value;
        this->locked_set_value(std::move(copied));
    }

   private:
    T _value;
    std::mutex _set_mutex;
};

template <typename T>
holder<T>::holder(T value) : sender<T>(std::make_shared<impl>(std::move(value))) {
}

template <typename T>
holder<T>::holder(std::shared_ptr<impl> &&impl) : sender<T>(std::move(impl)) {
}

template <typename T>
holder<T>::~holder() = default;

template <typename T>
void holder<T>::set_value(T value) {
    this->template impl_ptr<impl>()->locked_set_value(std::move(value));
}

template <typename T>
[[nodiscard]] T const &holder<T>::raw() const { return this->template impl_ptr<impl>()->value(); }

template <typename T>
[[nodiscard]] T &holder<T>::raw() { return this->template impl_ptr<impl>()->value(); }

template <typename T>
chain_sync_t<T> holder<T>::chain() const {
    return this->template impl_ptr<impl>()->chain_sync();
}

template <typename T>
receivable_ptr<T> holder<T>::receivable() {
    return this->template impl_ptr<typename chaining::receivable<T>>();
}

template <typename T>
std::shared_ptr<weakable_impl> holder<T>::weakable_impl_ptr() const {
    return this->template impl_ptr<impl>();
}

template <typename T>
std::shared_ptr<holder<T>> holder<T>::make_shared(T value) {
    return std::shared_ptr<holder<T>>(new holder<T>{std::move(value)});
}
}  // namespace yas::chaining::value
