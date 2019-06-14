//
//  yas_chaining_receiver.h
//

#pragma once

#include <cpp_utils/yas_base.h>
#include "yas_chaining_receiver.h"

namespace yas::chaining {
template <typename T = std::nullptr_t>
struct[[nodiscard]] perform_receiver : base, receiver<T> {
    class impl;

    perform_receiver(std::function<void(T const &)>);
    perform_receiver(std::function<void(void)>);
    perform_receiver(std::nullptr_t);

    ~perform_receiver() final;

    [[nodiscard]] receivable<T> receivable() override;

   private:
    chaining::receivable<T> _receivable = nullptr;
};
}  // namespace yas::chaining

#include "yas_chaining_perform_receiver_private.h"
