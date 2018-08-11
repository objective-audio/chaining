//
//  yas_chaining_sender_private.h
//

#pragma once

namespace yas::chaining {
template <typename T>
struct sender_base<T>::impl : base::impl, sender_chainable<T>::impl {
    std::unordered_map<std::uintptr_t, weak<joint<T>>> joints;

    void erase_joint(std::uintptr_t const key) override {
        this->joints.erase(key);
    }

    void sync(std::uintptr_t const key) override {
    }

    template <bool Syncable>
    chain<T, T, T, Syncable> begin(sender_base<T> &sender) {
        chaining::joint<T> joint{to_weak(sender)};
        this->joints.insert(std::make_pair(joint.identifier(), to_weak(joint)));
        return joint.template begin<Syncable>();
    }
};

template <typename T>
sender_base<T>::sender_base(std::shared_ptr<impl> &&impl) : base(std::move(impl)) {
}

template <typename T>
sender_base<T>::sender_base(std::nullptr_t) : base(nullptr) {
}

template <typename T>
sender_chainable<T> sender_base<T>::chainable() {
    if (!this->_chainable) {
        this->_chainable = sender_chainable<T>{impl_ptr<typename sender_chainable<T>::impl>()};
    }
    return this->_chainable;
}
}  // namespace yas::chaining
