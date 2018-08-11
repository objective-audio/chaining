//
//  yas_chaining_sender.h
//

#pragma once

namespace yas::chaining {
template <typename T>
struct sender_base : base {
    class impl;

    sender_base(std::nullptr_t);

    [[nodiscard]] sender_chainable<T> chainable();

   protected:
    sender_base(std::shared_ptr<impl> &&);

   private:
    sender_chainable<T> _chainable = nullptr;
};
}  // namespace yas::chaining

#include "yas_chaining_sender_private.h"
