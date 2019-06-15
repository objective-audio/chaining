//
//  yas_chaining_notifier.h
//

#pragma once

#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining {
template <typename T>
struct notifier : sender<T>, receiver<T> {
    class impl;

    notifier();
    notifier(std::nullptr_t);

    void notify(T const &);

    [[nodiscard]] chain_unsync_t<T> chain() const;

    [[nodiscard]] chaining::receivable<T> receivable() override;

   protected:
    notifier(std::shared_ptr<impl> &&);

   private:
    chaining::receivable<T> _receivable = nullptr;
};
}  // namespace yas::chaining

#include "yas_chaining_notifier_private.h"
