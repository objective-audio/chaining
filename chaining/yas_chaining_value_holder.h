//
//  yas_chaining_value_holder.h
//

#pragma once

#include <cpp_utils/yas_weakable.h>
#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining::value {
template <typename T>
struct holder final : sender<T>, receiver<T>, weakable<holder<T>> {
    class impl;

    holder(T);
    explicit holder(std::shared_ptr<impl> &&);

    ~holder();

    void set_value(T);

    [[nodiscard]] T const &raw() const;
    [[nodiscard]] T &raw();

    [[nodiscard]] chain_sync_t<T> chain() const;

    [[nodiscard]] receivable_ptr<T> receivable() override;

    std::shared_ptr<weakable_impl> weakable_impl_ptr() const override;

    static std::shared_ptr<holder> make_shared(T);
};
}  // namespace yas::chaining::value

#include "yas_chaining_value_holder_private.h"
