//
//  yas_chaining_receiver_protocol.h
//

#pragma once

#include <cpp_utils/yas_protocol.h>

namespace yas::chaining {
template <typename T = std::nullptr_t>
struct [[nodiscard]] receivable {
    virtual ~receivable() = default;

    virtual void receive_value(T const &) = 0;
};

template <typename T = std::nullptr_t>
using receivable_ptr = std::shared_ptr<receivable<T>>;
}  // namespace yas::chaining
