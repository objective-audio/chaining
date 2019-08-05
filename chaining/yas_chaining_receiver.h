//
//  yas_chaining_receiver.h
//

#pragma once

#include "yas_chaining_receiver_protocol.h"

namespace yas::chaining {
struct any_receiver {
    virtual ~any_receiver() = default;
};

template <typename T = std::nullptr_t>
struct receiver : any_receiver {
    using ReceiveType = T;

    virtual receivable_ptr<T> receivable() = 0;

   protected:
    receiver() = default;
};

template <typename T>
using is_base_of_receiver = std::is_base_of<any_receiver, T>;
template <typename T, typename V = void>
using enable_if_base_of_receiver_t = typename std::enable_if_t<is_base_of_receiver<T>::value, V>;
template <typename T, typename V = void>
using disable_if_base_of_receiver_t = typename std::enable_if_t<!is_base_of_receiver<T>::value, V>;
}  // namespace yas::chaining
