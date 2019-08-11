//
//  yas_chaining_sender.h
//

#pragma once

#include <memory>

namespace yas::chaining {
template <typename T>
class sendable;
class any_joint;

struct any_sender {};

template <typename T>
struct sender : any_sender, sendable<T> {
    using SendType = T;

    [[nodiscard]] std::shared_ptr<sendable<T>> sendable();

    bool operator==(sender const &rhs) const;
    bool operator!=(sender const &rhs) const;

    uintptr_t identifier() const;

   protected:
    virtual bool is_equal(sender<T> const &rhs) const;

    void fetch_for(any_joint const &joint) override;
};

template <typename T>
using is_base_of_sender = std::is_base_of<any_sender, T>;
template <typename T, typename V = void>
using enable_if_base_of_sender_t = typename std::enable_if_t<is_base_of_sender<T>::value, V>;
template <typename T, typename V = void>
using disable_if_base_of_sender_t = typename std::enable_if_t<!is_base_of_sender<T>::value, V>;
}  // namespace yas::chaining

#include "yas_chaining_sender_private.h"
