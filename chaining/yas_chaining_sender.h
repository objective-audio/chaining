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

struct any_sender {
    virtual ~any_sender() = default;
};

template <typename T>
struct sender : any_sender, std::enable_shared_from_this<sender<T>> {
    using send_type = T;

    virtual chain_unsync_t<T> chain_unsync() const;
    virtual chain_sync_t<T> chain_sync() const;

   protected:
    virtual void fetch_for(any_joint const &joint) const = 0;

    virtual void broadcast(T const &value) const;
    virtual void erase_joint(std::uintptr_t const key) const;
    virtual void send_value_to_target(T const &value, std::uintptr_t const key) const;

   private:
    mutable std::vector<std::weak_ptr<joint<T>>> _joints;

    template <bool Syncable>
    chain<T, T, Syncable> _chain() const;

    template <typename U>
    friend class joint;
};

template <typename T>
using is_base_of_sender = std::is_base_of<any_sender, T>;
template <typename T, typename V = void>
using enable_if_base_of_sender_t = typename std::enable_if_t<is_base_of_sender<T>::value, V>;
template <typename T, typename V = void>
using disable_if_base_of_sender_t = typename std::enable_if_t<!is_base_of_sender<T>::value, V>;
}  // namespace yas::chaining

#include "yas_chaining_sender_private.h"
