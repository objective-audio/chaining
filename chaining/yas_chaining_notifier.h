//
//  yas_chaining_notifier.h
//

#pragma once

#include "yas_chaining_sender.h"

namespace yas::chaining {
template <typename T>
struct receiver;

template <typename T>
struct notifier : sender<T> {
    class impl;

    notifier();
    notifier(std::nullptr_t);

    void notify(T const &);

    [[nodiscard]] chain_unsync_t<T> chain() const;

    [[nodiscard]] receiver<T> &receiver();

   protected:
    notifier(std::shared_ptr<impl> &&);
};
}  // namespace yas::chaining

#include "yas_chaining_notifier_private.h"
