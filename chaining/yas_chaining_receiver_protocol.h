//
//  yas_chaining_receiver_protocol.h
//

#pragma once

#include "yas_protocol.h"

namespace yas::chaining {
template <typename T>
class receiver;

template <typename T>
struct receivable : protocol {
    struct impl : protocol::impl {
        virtual void receive_value(T const &) = 0;
    };

    explicit receivable(std::shared_ptr<impl>);
    receivable(std::nullptr_t);

    void receive_value(T const &);
};
}  // namespace yas::chaining

#include "yas_chaining_receiver_protocol_private.h"
