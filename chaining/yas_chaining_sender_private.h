//
//  yas_chaining_sender_private.h
//

#pragma once

#include <cpp_utils/yas_stl_utils.h>

#include <vector>

namespace yas::chaining {
template <typename T>
void sender<T>::broadcast(T const &value) const {
    for (std::weak_ptr<joint<T>> const &weak_joint : this->_joints) {
        if (joint_ptr<T> joint = weak_joint.lock()) {
            joint->call_first(value);
        }
    }
}

template <typename T>
void sender<T>::erase_joint(std::uintptr_t const key) const {
    erase_if(this->_joints, [key](auto const &weak_joint) {
        if (auto joint = weak_joint.lock()) {
            return joint->identifier() == key;
        } else {
            return false;
        }
    });
}

template <typename T>
chain_unsync_t<T> sender<T>::chain_unsync() const {
    return this->_chain<false>();
}

template <typename T>
chain_sync_t<T> sender<T>::chain_sync() const {
    return this->_chain<true>();
}

template <typename T>
void sender<T>::send_value_to_target(T const &value, std::uintptr_t const key) const {
    for (std::weak_ptr<joint<T>> const &weak_joint : this->_joints) {
        if (joint_ptr<T> joint = weak_joint.lock(); joint && joint->identifier() == key) {
            joint->call_first(value);
        }
    }
}

template <typename T>
template <bool Syncable>
chain<T, T, Syncable> sender<T>::_chain() const {
    auto joint = make_joint(to_weak(this->shared_from_this()));
    this->_joints.emplace_back(to_weak(joint));
    return chaining::chain<T, T, Syncable>{std::move(joint)};
}
}  // namespace yas::chaining
