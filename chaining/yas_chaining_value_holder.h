//
//  yas_chaining_value_holder.h
//

#pragma once

#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining::value {
template <typename T>
struct holder : sender<T>, receiver<T> {
    class impl;

    holder(T);
    holder(std::nullptr_t);

    ~holder() final;

    void set_value(T);

    [[nodiscard]] T const &raw() const;
    [[nodiscard]] T &raw();

    [[nodiscard]] chain_sync_t<T> chain() const;

    [[nodiscard]] receivable<T> receivable() override;
};
}  // namespace yas::chaining::value

#include "yas_chaining_value_holder_private.h"
