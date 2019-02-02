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
struct holder : sender<T> {
    class impl;

    holder(T);
    holder(std::nullptr_t);

    ~holder() final;

    [[nodiscard]] T const &value() const;
    [[nodiscard]] T &value();
    void set_value(T);

    [[nodiscard]] T const &raw() const;

    [[nodiscard]] chain_sync_t<T> chain();

    [[nodiscard]] receiver<T> &receiver();
};
}  // namespace yas::chaining

#include "yas_chaining_holder_private.h"
