//
//  yas_chaining_notifier.h
//

#pragma once

#include <cpp_utils/yas_weakable.h>
#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining {
template <typename T>
struct notifier final : sender<T>, receiver<T>, weakable<notifier<T>> {
    class impl;

    notifier();
    notifier(std::shared_ptr<impl> &&);

    void notify(T const &);

    [[nodiscard]] chain_unsync_t<T> chain() const;

    [[nodiscard]] chaining::receivable_ptr<T> receivable() override;

    std::shared_ptr<weakable_impl> weakable_impl_ptr() const override;
};
}  // namespace yas::chaining

#include "yas_chaining_notifier_private.h"
