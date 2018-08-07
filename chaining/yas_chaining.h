//
//  yas_chaining.h
//

#pragma once

#include <functional>
#include "yas_any.h"
#include "yas_chaining_protocol.h"
#include "yas_type_traits.h"
#include "yas_types.h"

namespace yas::chaining {
template <typename T = std::nullptr_t>
struct receiver : base {
    class impl;

    receiver(std::function<void(T const &)>);
    receiver(std::function<void(void)>);
    receiver(std::nullptr_t);

    ~receiver() final;

    [[nodiscard]] receiver_chainable<T> chainable();

   private:
    receiver_chainable<T> _chainable = nullptr;
};

template <typename T>
struct sender_base : base {
    class impl;

    sender_base(std::nullptr_t);

    [[nodiscard]] sender_chainable<T> chainable();

   protected:
    sender_base(std::shared_ptr<impl> &&);

   private:
    sender_chainable<T> _chainable = nullptr;
};

template <typename T>
struct notifier : sender_base<T> {
    class impl;

    notifier();
    notifier(std::nullptr_t);

    void notify(T const &);

    [[nodiscard]] node<T, T, T, false> chain();

    [[nodiscard]] receiver<T> &receiver();

   protected:
    notifier(std::shared_ptr<impl> &&);
};

template <typename T>
struct fetcher : sender_base<T> {
    class impl;

    fetcher(std::function<opt_t<T>(void)>);
    fetcher(std::nullptr_t);

    void fetch() const;

    [[nodiscard]] node<T, T, T, true> chain();

    [[nodiscard]] receiver<> &receiver();
};

template <typename T>
struct holder : sender_base<T> {
    class impl;

    holder(T);
    holder(std::nullptr_t);

    ~holder() final;

    T const &value() const;
    T &value();
    void set_value(T);

    [[nodiscard]] node<T, T, T, true> chain();

    [[nodiscard]] receiver<T> &receiver();
};

struct observer : base {
    struct impl : base::impl {
        virtual void sync() = 0;
    };

    observer(std::nullptr_t) : base(nullptr) {
    }

    void sync() {
        impl_ptr<impl>()->sync();
    }

   protected:
    observer(std::shared_ptr<impl> &&impl) : base(std::move(impl)) {
    }
};

template <typename Begin>
struct typed_observer : observer {
    class impl;

    typed_observer(input<Begin>);
    typed_observer(std::nullptr_t);

    ~typed_observer() final;

    [[nodiscard]] chaining::input<Begin> &input();
};

template <typename Out, typename In, typename Begin, bool Syncable>
struct node : base {
    class impl;

    node(input<Begin>);
    // private
    node(input<Begin>, std::function<Out(In const &)>);
    node(std::nullptr_t);

    ~node() final;

    [[nodiscard]] auto normalize();

    [[nodiscard]] auto perform(std::function<void(Out const &)>);

    template <std::size_t N = 0, typename T>
    [[nodiscard]] auto receive(receiver<T> &);
    template <typename T, std::size_t N>
    [[nodiscard]] auto receive(std::array<receiver<T>, N>);
    template <typename T>
    [[nodiscard]] auto receive(std::vector<receiver<T>>);
    template <typename T>
    [[nodiscard]] auto receive(std::initializer_list<receiver<T>>);
    [[nodiscard]] auto receive_null(receiver<std::nullptr_t> &);

    [[nodiscard]] auto guard(std::function<bool(Out const &)>);

    template <typename F>
    [[nodiscard]] auto to(F);
    template <typename T>
    [[nodiscard]] auto to_value(T);
    [[nodiscard]] auto to_null();
    [[nodiscard]] auto to_tuple();

    template <typename SubIn, typename SubBegin, bool SubSyncable>
    [[nodiscard]] auto merge(node<Out, SubIn, SubBegin, SubSyncable>);

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
    [[nodiscard]] auto pair(node<SubOut, SubIn, SubBegin, SubSyncable>);

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
    [[nodiscard]] auto combine(node<SubOut, SubIn, SubBegin, SubSyncable>);

    [[nodiscard]] typed_observer<Begin> end();
    [[nodiscard]] typed_observer<Begin> sync();
};

template <typename T, bool Syncable>
using node_t = node<T, T, T, Syncable>;
}  // namespace yas::chaining

#include "yas_chaining_private.h"
