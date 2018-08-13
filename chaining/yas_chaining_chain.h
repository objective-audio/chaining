//
//  yas_chaining_chain.h
//

#pragma once

#include <functional>
#include "yas_chaining_observer.h"
#include "yas_chaining_receiver.h"
#include "yas_type_traits.h"

namespace yas::chaining {

template <typename Out, typename In, typename Begin, bool Syncable>
struct chain : base {
    class impl;

    chain(joint<Begin>);
    // private
    chain(joint<Begin>, std::function<Out(In const &)>);
    chain(std::nullptr_t);

    ~chain() final;

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
    [[nodiscard]] auto merge(chain<Out, SubIn, SubBegin, SubSyncable>);

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
    [[nodiscard]] auto pair(chain<SubOut, SubIn, SubBegin, SubSyncable>);

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
    [[nodiscard]] auto combine(chain<SubOut, SubIn, SubBegin, SubSyncable>);

    [[nodiscard]] typed_observer<Begin> end();
    [[nodiscard]] typed_observer<Begin> sync();
};

template <typename T, bool Syncable>
using chain_t = chain<T, T, T, Syncable>;
}  // namespace yas::chaining

#include "yas_chaining_chain_private.h"
