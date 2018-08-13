//
//  yas_chaining_sender_chainable.h
//

#pragma once

#include "yas_protocol.h"

namespace yas::chaining {
template <typename T>
struct sender_chainable : protocol {
    struct impl : protocol::impl {
        virtual void erase_joint(std::uintptr_t const) = 0;
        virtual void sync(std::uintptr_t const) = 0;
    };

    sender_chainable(std::shared_ptr<impl>);
    sender_chainable(std::nullptr_t);

    void erase_joint(std::uintptr_t const);
    void sync(std::uintptr_t const);
};
}  // namespace yas::chaining

#include "yas_chaining_sender_protocol_private.h"
