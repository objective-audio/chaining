//
//  yas_chaining_sender_private.h
//

#pragma once

#include <vector>
#include "yas_chaining_joint.h"
#include "yas_chaining_sender_protocol.h"
#include "yas_stl_utils.h"

namespace yas::chaining {
template <typename Out, typename In, typename Begin, bool Syncable>
class chain;

template <typename T>
struct sender<T>::impl : base::impl, sender_chainable<T>::impl {
    void broadcast(T const &value) override {
        for (weak<joint<T>> const &weak_joint : this->_joints) {
            if (joint<T> joint = weak_joint.lock()) {
                joint.call_first(value);
            }
        }
    }

    void send_value_to_target(T const &value, std::uintptr_t const key) override {
        for (weak<joint<T>> const &weak_joint : this->_joints) {
            if (weak_joint.identifier() == key) {
                if (joint<T> joint = weak_joint.lock()) {
                    joint.call_first(value);
                }
            }
        }
    }

    void erase_joint(std::uintptr_t const key) override {
        erase_if(this->_joints, [key](auto const &weak_joint) { return weak_joint.identifier() == key; });
    }

    void fetch_for(any_joint const &joint) override {
    }

    template <bool Syncable>
    chain<T, T, T, Syncable> chain(sender<T> &sender) {
        chaining::joint<T> joint{to_weak(sender)};
        this->_joints.emplace_back(to_weak(joint));
        return joint.template chain<Syncable>();
    }

   private:
    std::vector<weak<joint<T>>> _joints;
};

template <typename T>
sender<T>::sender(std::shared_ptr<impl> &&impl) : base(std::move(impl)) {
}

template <typename T>
sender<T>::sender(std::nullptr_t) : base(nullptr) {
}

template <typename T>
sender_chainable<T> sender<T>::chainable() {
    if (!this->_chainable) {
        this->_chainable = sender_chainable<T>{impl_ptr<typename sender_chainable<T>::impl>()};
    }
    return this->_chainable;
}
}  // namespace yas::chaining
