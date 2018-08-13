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
struct holder : sender_base<T> {
    class impl;

    holder(T);
    holder(std::nullptr_t);

    ~holder() final;

    T const &value() const;
    T &value();
    void set_value(T);

    [[nodiscard]] chain<T, T, T, true> chain();

    [[nodiscard]] receiver<T> &receiver();
};
}  // namespace yas::chaining

#include "yas_chaining_holder_private.h"
