//
//  yas_chaining_joint_private.h
//

#pragma once

#include <vector>
#include "yas_any.h"

namespace yas::chaining {
template <typename T>
struct joint<T>::impl : joint_base::impl {
    weak<sender_base<T>> _weak_sender;

    impl(weak<sender_base<T>> &&weak_sender) : _weak_sender(std::move(weak_sender)) {
    }

    void call_first(T const &value) {
        if (this->_handlers.size() > 0) {
            this->_handlers.front().template get<std::function<void(T const &)>>()(value);
        } else {
            std::runtime_error("handler not found. must call the end.");
        }
    }

    void broadcast() override {
        if (auto sender = this->_weak_sender.lock()) {
            sender.chainable().sync(this->identifier());
        }

        for (auto &sub_joint : this->_sub_joints) {
            sub_joint.broadcast();
        }
    }

    void push_handler(yas::any &&handler) {
        this->_handlers.emplace_back(std::move(handler));
    }

    yas::any handler(std::size_t const idx) {
        return this->_handlers.at(idx);
    }

    std::size_t handlers_size() {
        return this->_handlers.size();
    }

    void add_sub_joint(joint_base &&sub_joint) {
        this->_sub_joints.emplace_back(std::move(sub_joint));
    }

   private:
    std::vector<yas::any> _handlers;
    std::vector<joint_base> _sub_joints;
};

template <typename T>
joint<T>::joint(weak<sender_base<T>> weak_sender) : joint_base(std::make_shared<impl>(std::move(weak_sender))) {
}

template <typename T>
joint<T>::joint(std::nullptr_t) : joint_base(nullptr) {
}

template <typename T>
joint<T>::~joint() {
    if (impl_ptr() && impl_ptr().unique()) {
        if (auto sender = impl_ptr<impl>()->_weak_sender.lock()) {
            sender.chainable().erase_joint(this->identifier());
        }
        impl_ptr().reset();
    }
}

template <typename T>
void joint<T>::call_first(T const &value) {
    impl_ptr<impl>()->call_first(value);
}

template <typename T>
template <bool Syncable>
chain<T, T, T, Syncable> joint<T>::chain() {
    return chaining::chain<T, T, T, Syncable>(*this);
}

template <typename T>
template <typename P>
void joint<T>::push_handler(std::function<void(P const &)> handler) {
    impl_ptr<impl>()->push_handler(std::move(handler));
}

template <typename T>
std::size_t joint<T>::handlers_size() const {
    return impl_ptr<impl>()->handlers_size();
}

template <typename T>
template <typename P>
std::function<void(P const &)> const &joint<T>::handler(std::size_t const idx) const {
    return impl_ptr<impl>()->handler(idx).template get<std::function<void(P const &)>>();
}

template <typename T>
void joint<T>::add_sub_joint(joint_base sub_joint) {
    impl_ptr<impl>()->add_sub_joint(std::move(sub_joint));
}
}  // namespace yas::chaining
