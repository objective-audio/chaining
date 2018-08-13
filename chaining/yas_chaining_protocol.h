//
//  yas_chaining_protocol.h
//

#pragma once

#include "yas_base.h"
#include "yas_chaining_sender_chainable.h"
#include "yas_protocol.h"

namespace yas::chaining {
template <typename Out, typename In, typename Begin, bool Syncable>
class chain;
template <typename T>
class receiver;

template <typename T>
struct output : base {
    class impl;

    output(weak<receiver<T>>);
    output(std::nullptr_t);

    void output_value(T const &);
};

template <typename T>
struct receiver_chainable : protocol {
    struct impl : protocol::impl {
        virtual output<T> make_output() = 0;
        virtual void receive_value(T const &) = 0;
    };

    explicit receiver_chainable(std::shared_ptr<impl>);
    receiver_chainable(std::nullptr_t);

    void receive_value(T const &);
    output<T> make_output();
};

}  // namespace yas::chaining

#include "yas_chaining_output_private.h"
