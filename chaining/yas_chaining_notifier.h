//
//  yas_chaining_notifier.h
//

#pragma once

#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining {
template <typename T>
struct notifier final : sender<T>, receiver<T> {
    void notify(T const &);

    [[nodiscard]] chain_unsync_t<T> chain();

    void receive_value(T const &) override;

   private:
    std::mutex _send_mutex;

    notifier();

   public:
    static std::shared_ptr<notifier> make_shared();
};
}  // namespace yas::chaining

#include "yas_chaining_notifier_private.h"
