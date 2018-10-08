//
//  yas_chaining_sender.h
//

#pragma once

#include "yas_base.h"

namespace yas::chaining {
template <typename T>
class sendable;

struct any_sender {};

template <typename T>
struct sender : base, any_sender {
    class impl;

    sender(std::nullptr_t);

    sendable<T> sendable();

   protected:
    sender(std::shared_ptr<impl> &&);

   private:
    chaining::sendable<T> _sendable = nullptr;
};

template <typename T>
using is_base_of_sender = std::is_base_of<any_sender, T>;
template <typename T, typename V = void>
using enable_if_base_of_sender_t = typename std::enable_if_t<is_base_of_sender<T>::value, V>;
template <typename T, typename V = void>
using disable_if_base_of_sender_t = typename std::enable_if_t<!is_base_of_sender<T>::value, V>;
}  // namespace yas::chaining

#include "yas_chaining_sender_private.h"
