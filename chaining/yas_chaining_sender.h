//
//  yas_chaining_sender.h
//

#pragma once

#include "yas_base.h"

namespace yas::chaining {
template <typename T>
class sender_chainable;

template <typename T>
struct sender : base {
    class impl;

    sender(std::nullptr_t);

    [[nodiscard]] sender_chainable<T> chainable();

   protected:
    sender(std::shared_ptr<impl> &&);

   private:
    sender_chainable<T> _chainable = nullptr;
};
}  // namespace yas::chaining

#include "yas_chaining_sender_private.h"
