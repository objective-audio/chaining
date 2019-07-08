//
//  yas_chaining_sender_private.h
//

#pragma once

#include <cpp_utils/yas_stl_utils.h>
#include <vector>
#include "yas_chaining_joint.h"
#include "yas_chaining_sender_protocol.h"

namespace yas::chaining {
template <typename T>
struct sender<T>::impl : chaining::sendable<T>, std::enable_shared_from_this<chaining::sendable<T>> {
    void broadcast(T const &value) override {
        for (std::weak_ptr<joint<T>> const &weak_joint : this->_joints) {
            if (joint_ptr<T> joint = weak_joint.lock()) {
                joint->call_first(value);
            }
        }
    }

    void send_value_to_target(T const &value, std::uintptr_t const key) override {
        for (std::weak_ptr<joint<T>> const &weak_joint : this->_joints) {
            if (joint_ptr<T> joint = weak_joint.lock(); joint && joint->identifier() == key) {
                joint->call_first(value);
            }
        }
    }

    void erase_joint(std::uintptr_t const key) override {
        erase_if(this->_joints, [key](auto const &weak_joint) {
            if (auto joint = weak_joint.lock()) {
                return joint->identifier() == key;
            } else {
                return false;
            }
        });
    }

    void fetch_for(any_joint const &joint) override {
    }

    chain_unsync_t<T> chain_unsync() override {
        return this->_chain<false>();
    }

    chain_sync_t<T> chain_sync() override {
        return this->_chain<true>();
    }

    uintptr_t identifier() const {
        return reinterpret_cast<uintptr_t>(this);
    }

    virtual bool is_equal(std::shared_ptr<impl> const &rhs) const {
        return this->identifier() == rhs->identifier();
    }

   private:
    std::vector<std::weak_ptr<joint<T>>> _joints;

    template <bool Syncable>
    chain<T, T, Syncable> _chain() {
        auto joint = make_joint(to_weak(this->shared_from_this()));
        this->_joints.emplace_back(to_weak(joint));
        return chaining::chain<T, T, Syncable>{std::move(joint)};
    }
};

template <typename T>
sender<T>::sender(std::shared_ptr<impl> &&impl) : _impl(std::move(impl)) {
}

template <typename T>
std::shared_ptr<sendable<T>> sender<T>::sendable() {
    return this->_impl;
}

template <typename T>
bool sender<T>::operator==(sender const &rhs) const {
    return _impl && rhs._impl && (_impl == rhs._impl || _impl->is_equal(rhs._impl));
}

template <typename T>
bool sender<T>::operator!=(sender const &rhs) const {
    return !_impl || !rhs._impl || (_impl != rhs._impl && !_impl->is_equal(rhs._impl));
}

template <typename T>
bool sender<T>::operator==(std::nullptr_t) const {
    return _impl == nullptr;
}

template <typename T>
bool sender<T>::operator!=(std::nullptr_t) const {
    return _impl != nullptr;
}

template <typename T>
template <typename Impl>
std::shared_ptr<Impl> sender<T>::impl_ptr() const {
    return std::dynamic_pointer_cast<Impl>(_impl);
}
}  // namespace yas::chaining
