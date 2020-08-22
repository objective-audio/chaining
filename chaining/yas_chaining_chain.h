//
//  yas_chaining_chain.h
//

#pragma once

#include <chaining/yas_chaining_receiver.h>
#include <cpp_utils/yas_type_traits.h>

#include <functional>

namespace yas::chaining {
template <typename T>
class observer;

template <typename Out, typename Begin, bool Syncable>
struct [[nodiscard]] chain final {
    class impl;

    chain(joint_ptr<Begin>);

    [[nodiscard]] chain<Out, Begin, Syncable> perform(std::function<void(Out const &)>);

    template <std::size_t N = 0, typename T>
    [[nodiscard]] chain<Out, Begin, Syncable> send_to(std::shared_ptr<T> const &);
    template <typename T, std::size_t N>
    [[nodiscard]] chain<Out, Begin, Syncable> send_to(std::array<std::shared_ptr<T>, N> const &);
    template <typename T>
    [[nodiscard]] chain<Out, Begin, Syncable> send_to(std::vector<std::shared_ptr<T>> const &);
    template <typename T>
    [[nodiscard]] chain<Out, Begin, Syncable> send_to(std::initializer_list<std::shared_ptr<T>> const &);
    [[nodiscard]] chain<Out, Begin, Syncable> send_null_to(std::shared_ptr<receiver<std::nullptr_t>> const &);

    [[nodiscard]] chain<Out, Begin, Syncable> guard(std::function<bool(Out const &)>);

    template <typename F>
    [[nodiscard]] chain<return_t<F>, Begin, Syncable> to(F);
    template <typename T>
    [[nodiscard]] chain<T, Begin, Syncable> to_value(T);
    [[nodiscard]] chain<std::nullptr_t, Begin, Syncable> to_null();
    [[nodiscard]] auto to_tuple();

    template <typename SubBegin, bool SubSyncable>
    [[nodiscard]] chain<Out, Begin, Syncable | SubSyncable> merge(chain<Out, SubBegin, SubSyncable> &&);

    template <typename SubOut, typename SubBegin, bool SubSyncable>
    [[nodiscard]] chain<opt_pair_t<Out, SubOut>, Begin, Syncable | SubSyncable> pair(
        chain<SubOut, SubBegin, SubSyncable> &&);

    template <typename SubOut, typename SubBegin, bool SubSyncable>
    [[nodiscard]] auto combine(chain<SubOut, SubBegin, SubSyncable> &&);

    [[nodiscard]] std::shared_ptr<observer<Begin>> end();
    [[nodiscard]] std::shared_ptr<observer<Begin>> sync();

    std::unique_ptr<impl> _impl;
};
}  // namespace yas::chaining

#include <chaining/yas_chaining_chain_private.h>
