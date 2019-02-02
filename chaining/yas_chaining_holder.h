//
//  yas_chaining_holder.h
//

#pragma once

#include "yas_chaining_sender.h"

namespace yas::chaining {
template <typename Out, typename In, typename Begin, bool Syncable>
class chain;
template <typename T>
class receiver;

template <typename T>
struct immutable_holder : sender<T> {
    class impl;

    [[nodiscard]] T const &value() const;

    chain_sync_t<T> chain();

   protected:
    immutable_holder(std::shared_ptr<impl> &&);
    immutable_holder(std::nullptr_t);
};

template <typename T>
struct holder : immutable_holder<T> {
    holder(T);
    holder(std::nullptr_t);

    ~holder() final;

    [[nodiscard]] T const &value() const;
    [[nodiscard]] T &value();
    void set_value(T);

    [[nodiscard]] T const &raw() const;

    receiver<T> &receiver();

   private:
    using immutable_impl = typename immutable_holder<T>::impl;
};
}  // namespace yas::chaining

#include "yas_chaining_holder_private.h"
