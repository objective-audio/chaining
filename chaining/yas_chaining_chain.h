//
//  yas_chaining_chain.h
//

#pragma once

#include <cpp_utils/yas_base.h>
#include <cpp_utils/yas_type_traits.h>
#include <functional>

namespace yas::chaining {
template <typename Begin>
struct observer;
template <typename T>
struct receiver;

template <typename Out, typename Begin, bool Syncable>
struct[[nodiscard]] chain : base {
    class impl;

    chain(joint<Begin>);
    chain(std::nullptr_t);

    ~chain() final;

    [[nodiscard]] chain<Out, Begin, Syncable> perform(std::function<void(Out const &)>);

    template <std::size_t N = 0, typename T>
    [[nodiscard]] chain<Out, Begin, Syncable> send_to(receiver<T> &);
    [[nodiscard]] chain<Out, Begin, Syncable> send_null(receiver<std::nullptr_t> &);

    [[nodiscard]] chain<Out, Begin, Syncable> guard(std::function<bool(Out const &)>);

    template <typename F>
    [[nodiscard]] chain<return_t<F>, Begin, Syncable> to(F);
    template <typename T>
    [[nodiscard]] chain<T, Begin, Syncable> to_value(T);
    [[nodiscard]] chain<std::nullptr_t, Begin, Syncable> to_null();
    [[nodiscard]] auto to_tuple();

    template <typename SubBegin, bool SubSyncable>
    [[nodiscard]] chain<Out, Begin, Syncable | SubSyncable> merge(chain<Out, SubBegin, SubSyncable>);

    template <typename SubOut, typename SubBegin, bool SubSyncable>
    [[nodiscard]] chain<opt_pair_t<Out, SubOut>, Begin, Syncable | SubSyncable> pair(
        chain<SubOut, SubBegin, SubSyncable>);

    template <typename SubOut, typename SubBegin, bool SubSyncable>
    [[nodiscard]] auto combine(chain<SubOut, SubBegin, SubSyncable>);

    [[nodiscard]] observer<Begin> end();
    [[nodiscard]] observer<Begin> sync();
};
}  // namespace yas::chaining

#include "yas_chaining_chain_private.h"
