//
//  yas_chaining_sender_protocol.h
//

#pragma once

#include <cpp_utils/yas_protocol.h>
#include "yas_chaining_joint.h"
#include "yas_chaining_types.h"

namespace yas::chaining {
template <typename T>
struct sendable : std::enable_shared_from_this<chaining::sendable<T>> {
    virtual ~sendable() = default;

    virtual void fetch_for(any_joint const &) = 0;

    void broadcast(T const &value) {
        for (std::weak_ptr<joint<T>> const &weak_joint : this->_joints) {
            if (joint_ptr<T> joint = weak_joint.lock()) {
                joint->call_first(value);
            }
        }
    }

    void send_value_to_target(T const &value, std::uintptr_t const key) {
        for (std::weak_ptr<joint<T>> const &weak_joint : this->_joints) {
            if (joint_ptr<T> joint = weak_joint.lock(); joint && joint->identifier() == key) {
                joint->call_first(value);
            }
        }
    }

    void erase_joint(std::uintptr_t const key) {
        erase_if(this->_joints, [key](auto const &weak_joint) {
            if (auto joint = weak_joint.lock()) {
                return joint->identifier() == key;
            } else {
                return false;
            }
        });
    }

    chain_unsync_t<T> chain_unsync() {
        return this->_chain<false>();
    }

    chain_sync_t<T> chain_sync() {
        return this->_chain<true>();
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
}  // namespace yas::chaining
