//
//  yas_chaining_receiver.h
//

#pragma once

#include "yas_base.h"

namespace yas::chaining {
template <typename T>
struct receivable;

template <typename T = std::nullptr_t>
struct[[nodiscard]] receiver : base {
    class impl;

    receiver(std::function<void(T const &)>);
    receiver(std::function<void(void)>);
    receiver(std::nullptr_t);

    ~receiver() final;

    receivable<T> receivable();

   private:
    chaining::receivable<T> _receivable = nullptr;
};
}  // namespace yas::chaining

#include "yas_chaining_receiver_private.h"
