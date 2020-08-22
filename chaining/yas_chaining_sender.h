//
//  yas_chaining_sender.h
//

#pragma once

#include <chaining/yas_chaining_joint.h>
#include <chaining/yas_chaining_types.h>
#include <cpp_utils/yas_stl_utils.h>

#include <memory>

namespace yas::chaining {
class any_joint;

struct any_sender {
    virtual ~any_sender() = default;
};

template <typename T>
struct sender_protocol : any_sender {
    virtual chain_unsync_t<T> chain_unsync() const = 0;
    virtual chain_sync_t<T> chain_sync() const = 0;
    virtual void fetch_for(any_joint const &joint) const = 0;
    virtual void broadcast(T const &value) const = 0;
    virtual void erase_joint(std::uintptr_t const key) const = 0;
    virtual void send_value_to_target(T const &value, std::uintptr_t const key) const = 0;
};

template <typename T>
struct sender : sender_protocol<T>, std::enable_shared_from_this<sender<T>> {
    using send_type = T;

    virtual chain_unsync_t<T> chain_unsync() const override;
    virtual chain_sync_t<T> chain_sync() const override;

   protected:
    virtual void fetch_for(any_joint const &joint) const override = 0;
    virtual void broadcast(T const &value) const override;
    virtual void erase_joint(std::uintptr_t const key) const override;
    virtual void send_value_to_target(T const &value, std::uintptr_t const key) const override;

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

#include <chaining/yas_chaining_sender_private.h>
