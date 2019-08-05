//
//  yas_chaining_receiver.h
//

#pragma once

#include "yas_chaining_receiver.h"

namespace yas::chaining {
template <typename T = std::nullptr_t>
struct[[nodiscard]] perform_receiver final : receiver<T> {
    class impl;

    [[nodiscard]] receivable_ptr<T> receivable() override;

   private:
    std::shared_ptr<impl> _impl;

    perform_receiver(std::function<void(T const &)> const &);
    perform_receiver(std::function<void(T const &)> &&);

   public:
    static std::shared_ptr<perform_receiver> make_shared(std::function<void(T const &)> const &);
    static std::shared_ptr<perform_receiver> make_shared(std::function<void(T const &)> &&);
    static std::shared_ptr<perform_receiver> make_shared(std::function<void(void)> const &);
    static std::shared_ptr<perform_receiver> make_shared(std::function<void(void)> &&);
};
}  // namespace yas::chaining

#include "yas_chaining_perform_receiver_private.h"
