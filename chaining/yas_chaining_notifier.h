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

    notifier(std::shared_ptr<impl> &&);

    void notify(T const &);

    [[nodiscard]] chain_unsync_t<T> chain() const;

    void receive_value(T const &) override;

    std::shared_ptr<weakable_impl> weakable_impl_ptr() const override;

   private:
    notifier();

   public:
    static std::shared_ptr<notifier> make_shared();
};
}  // namespace yas::chaining

#include "yas_chaining_notifier_private.h"
