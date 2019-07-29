//
//  yas_chaining_sender.h
//

#pragma once

#include <memory>

namespace yas::chaining {
template <typename T>
class sendable;

struct any_sender {};

template <typename T>
struct sender : any_sender {
    using SendType = T;

    class impl;

    [[nodiscard]] std::shared_ptr<sendable<T>> sendable();

    bool operator==(sender const &rhs) const;
    bool operator!=(sender const &rhs) const;
    bool operator==(std::nullptr_t) const;
    bool operator!=(std::nullptr_t) const;

    template <typename Impl = impl>
    std::shared_ptr<Impl> impl_ptr() const;

   protected:
    sender(std::shared_ptr<impl> &&);

   private:
    std::shared_ptr<impl> _impl;
};

template <typename T>
using is_base_of_sender = std::is_base_of<any_sender, T>;
template <typename T, typename V = void>
using enable_if_base_of_sender_t = typename std::enable_if_t<is_base_of_sender<T>::value, V>;
template <typename T, typename V = void>
using disable_if_base_of_sender_t = typename std::enable_if_t<!is_base_of_sender<T>::value, V>;
}  // namespace yas::chaining

#include "yas_chaining_sender_private.h"
