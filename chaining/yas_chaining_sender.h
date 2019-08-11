//
//  yas_chaining_sender.h
//

#pragma once

#include <cpp_utils/yas_stl_utils.h>
#include <memory>
#include "yas_chaining_joint.h"
#include "yas_chaining_types.h"

namespace yas::chaining {
class any_joint;

struct any_sender {};

template <typename T>
struct sender : any_sender, std::enable_shared_from_this<sender<T>> {
    using SendType = T;

    uintptr_t identifier() const;

    virtual void fetch_for(any_joint const &joint);

    void broadcast(T const &value) {
        for (std::weak_ptr<joint<T>> const &weak_joint : this->_joints) {
            if (joint_ptr<T> joint = weak_joint.lock()) {
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

   protected:
    void send_value_to_target(T const &value, std::uintptr_t const key) {
        for (std::weak_ptr<joint<T>> const &weak_joint : this->_joints) {
            if (joint_ptr<T> joint = weak_joint.lock(); joint && joint->identifier() == key) {
                joint->call_first(value);
            }
        }
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
using is_base_of_sender = std::is_base_of<any_sender, T>;
template <typename T, typename V = void>
using enable_if_base_of_sender_t = typename std::enable_if_t<is_base_of_sender<T>::value, V>;
template <typename T, typename V = void>
using disable_if_base_of_sender_t = typename std::enable_if_t<!is_base_of_sender<T>::value, V>;
}  // namespace yas::chaining

#include "yas_chaining_sender_private.h"
