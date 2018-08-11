//
//  yas_chaining_receiver.h
//

#pragma once

#include "yas_base.h"

namespace yas::chaining {
template <typename T = std::nullptr_t>
struct receiver : base {
    class impl;

    receiver(std::function<void(T const &)>);
    receiver(std::function<void(void)>);
    receiver(std::nullptr_t);

    ~receiver() final;

    [[nodiscard]] receiver_chainable<T> chainable();

   private:
    receiver_chainable<T> _chainable = nullptr;
};
}  // namespace yas::chaining

#include "yas_chaining_receiver_private.h"
