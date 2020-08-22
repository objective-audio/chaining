//
//  yas_chaining_notifier.h
//

#pragma once

#include <chaining/yas_chaining_receiver.h>
#include <chaining/yas_chaining_sender.h>

namespace yas::chaining {
template <typename T>
class notifier;

template <typename T>
using notifier_ptr = std::shared_ptr<notifier<T>>;

template <typename T>
struct notifier final : sender<T>, receiver<T> {
    void notify(T const &);

    [[nodiscard]] chain_unsync_t<T> chain();

    void receive_value(T const &) override;

    static notifier_ptr<T> make_shared();

   private:
    std::mutex _send_mutex;

    notifier();

    notifier(notifier const &) = delete;
    notifier(notifier &&) = delete;
    notifier &operator=(notifier const &) = delete;
    notifier &operator=(notifier &&) = delete;

    void fetch_for(any_joint const &joint) const override;
};
}  // namespace yas::chaining

#include <chaining/yas_chaining_notifier_private.h>
