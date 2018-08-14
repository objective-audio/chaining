//
//  yas_chaining_receiver_chainable.h
//

#pragma once

#include "yas_protocol.h"

namespace yas::chaining {
template <typename T>
class receiver;

template <typename T>
struct receiver_chainable : protocol {
    struct impl : protocol::impl {
        virtual void receive_value(T const &) = 0;
    };

    explicit receiver_chainable(std::shared_ptr<impl>);
    receiver_chainable(std::nullptr_t);

    void receive_value(T const &);
};
}  // namespace yas::chaining

#include "yas_chaining_receiver_protocol_private.h"
