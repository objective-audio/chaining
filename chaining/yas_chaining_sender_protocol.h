//
//  yas_chaining_sender_protocol.h
//

#pragma once

#include "yas_protocol.h"

namespace yas::chaining {
class any_joint;

template <typename T>
struct sendable : protocol {
    struct impl : protocol::impl {
        virtual void broadcast(T const &) = 0;
        virtual void send_value_to_target(T const &, std::uintptr_t const) = 0;
        virtual void erase_joint(std::uintptr_t const) = 0;
        virtual void fetch_for(any_joint const &) = 0;
    };

    sendable(std::shared_ptr<impl>);
    sendable(std::nullptr_t);

    void broadcast(T const &);
    void send_value_to_target(T const &, std::uintptr_t const);
    void erase_joint(std::uintptr_t const);
    void fetch_for(any_joint const &);
};
}  // namespace yas::chaining

#include "yas_chaining_sender_protocol_private.h"
