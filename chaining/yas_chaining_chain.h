//
//  yas_chaining_chain.h
//

#pragma once

#include <functional>
#include "yas_base.h"

namespace yas::chaining {
template <typename Begin>
struct observer;
template <typename T>
struct receiver;
template <typename T>
struct joint;

template <typename Out, typename In, typename Begin, bool Syncable>
struct chain : base {
    class impl;

    chain(joint<Begin>);
    // private
    chain(joint<Begin>, std::function<Out(In const &)>);
    chain(std::nullptr_t);

    ~chain() final;

    [[nodiscard]] auto normalize();

    [[nodiscard]] chain<Out, In, Begin, Syncable> perform(std::function<void(Out const &)>);

    template <std::size_t N = 0, typename T>
    [[nodiscard]] chain<Out, In, Begin, Syncable> receive(receiver<T> &);
    template <typename T, std::size_t N>
    [[nodiscard]] chain<Out, In, Begin, Syncable> receive(std::array<receiver<T>, N>);
    template <typename T>
    [[nodiscard]] chain<Out, In, Begin, Syncable> receive(std::vector<receiver<T>>);
    template <typename T>
    [[nodiscard]] chain<Out, In, Begin, Syncable> receive(std::initializer_list<receiver<T>>);
    [[nodiscard]] chain<Out, In, Begin, Syncable> receive_null(receiver<std::nullptr_t> &);

    [[nodiscard]] chain<Out, Out, Begin, Syncable> guard(std::function<bool(Out const &)>);

    template <typename F>
    [[nodiscard]] auto to(F);
    template <typename T>
    [[nodiscard]] auto to_value(T);
    [[nodiscard]] auto to_null();
    [[nodiscard]] auto to_tuple();

    template <typename SubIn, typename SubBegin, bool SubSyncable>
    [[nodiscard]] chain<Out, Out, Begin, Syncable | SubSyncable> merge(chain<Out, SubIn, SubBegin, SubSyncable>);

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
    [[nodiscard]] auto pair(chain<SubOut, SubIn, SubBegin, SubSyncable>);

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
    [[nodiscard]] auto combine(chain<SubOut, SubIn, SubBegin, SubSyncable>);

    [[nodiscard]] observer<Begin> end();
    [[nodiscard]] observer<Begin> sync();
};

template <typename T>
using chain_syncable_t = chain<T, T, T, true>;
template <typename T>
using chain_unsyncable_t = chain<T, T, T, false>;
}  // namespace yas::chaining

#include "yas_chaining_chain_private.h"
