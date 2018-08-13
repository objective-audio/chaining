//
//  yas_chaining_protocol.h
//

#pragma once

#include "yas_base.h"
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

#include "yas_chaining_output_private.h"
