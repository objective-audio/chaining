//
//  yas_chaining_protocol.h
//

#pragma once

#include "yas_base.h"
#include "yas_protocol.h"

namespace yas::chaining {
template <typename Out, typename In, typename Begin, bool Syncable>
class node;
template <typename T>
class sender_base;
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

struct input_base : base {
    struct impl : base::impl {
        virtual void sync() = 0;
    };

    input_base(std::shared_ptr<impl> &&ptr) : base(std::move(ptr)) {
    }

    input_base(std::nullptr_t) : base(nullptr) {
    }

    void sync() {
        impl_ptr<impl>()->sync();
    }
};

template <typename T>
struct input : input_base {
    class impl;

    input(weak<sender_base<T>>);
    input(std::nullptr_t);

    ~input() final;

    void input_value(T const &);

    template <bool Syncable>
    [[nodiscard]] node<T, T, T, Syncable> begin();

    template <typename P>
    void push_handler(std::function<void(P const &)>);
    std::size_t handlers_size() const;
    template <typename P>
    std::function<void(P const &)> const &handler(std::size_t const) const;
    void add_sub_input(input_base sub_input);
};

template <typename T>
struct sender_chainable : protocol {
    struct impl : protocol::impl {
        virtual void erase_input(std::uintptr_t const) = 0;
        virtual void sync(std::uintptr_t const) = 0;
    };

    sender_chainable(std::shared_ptr<impl>);
    sender_chainable(std::nullptr_t);

    void erase_input(std::uintptr_t const);
    void sync(std::uintptr_t const);
};
}  // namespace yas::chaining
