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

    notifier(notifier const &) = delete;
    notifier(notifier &&) = delete;
    notifier &operator=(notifier const &) = delete;
    notifier &operator=(notifier &&) = delete;

    void fetch_for(any_joint const &joint) const override;

   public:
    static std::shared_ptr<notifier> make_shared();
};

template <typename T>
using notifier_ptr = std::shared_ptr<notifier<T>>;
}  // namespace yas::chaining

#include "yas_chaining_notifier_private.h"
