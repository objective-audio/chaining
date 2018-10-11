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
struct[[nodiscard]] chain : base {
    class impl;

    chain(joint<Begin>);
    // private
    chain(joint<Begin>, std::function<Out(In const &)>);
    chain(std::nullptr_t);

    ~chain() final;

    auto normalize();

    chain<Out, In, Begin, Syncable> perform(std::function<void(Out const &)>);

    template <std::size_t N = 0, typename T>
    chain<Out, In, Begin, Syncable> receive(receiver<T> &);
    template <typename T, std::size_t N>
    chain<Out, In, Begin, Syncable> receive(std::array<receiver<T>, N>);
    template <typename T>
    chain<Out, In, Begin, Syncable> receive(std::vector<receiver<T>>);
    template <typename T>
    chain<Out, In, Begin, Syncable> receive(std::initializer_list<receiver<T>>);
    chain<Out, In, Begin, Syncable> receive_null(receiver<std::nullptr_t> &);

    chain<Out, Out, Begin, Syncable> guard(std::function<bool(Out const &)>);

    template <typename F>
    auto to(F);
    template <typename T>
    auto to_value(T);
    auto to_null();
    auto to_tuple();

    template <typename SubIn, typename SubBegin, bool SubSyncable>
    chain<Out, Out, Begin, Syncable | SubSyncable> merge(chain<Out, SubIn, SubBegin, SubSyncable>);

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
    auto pair(chain<SubOut, SubIn, SubBegin, SubSyncable>);

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
    auto combine(chain<SubOut, SubIn, SubBegin, SubSyncable>);

    observer<Begin> end();
    observer<Begin> sync();
};

template <typename T>
using chain_syncable_t = chain<T, T, T, true>;
template <typename T, typename U>
using chain_relayed_syncable_t = chain<T, U, U, true>;
template <typename T, typename U>
using chain_normalized_syncable_t = chain<T, T, U, true>;
template <typename T>
using chain_unsyncable_t = chain<T, T, T, false>;
template <typename T, typename U>
using chain_relayed_unsyncable_t = chain<T, U, U, false>;
template <typename T, typename U>
using chain_normalized_unsyncable_t = chain<T, T, U, false>;
}  // namespace yas::chaining

#include "yas_chaining_chain_private.h"
