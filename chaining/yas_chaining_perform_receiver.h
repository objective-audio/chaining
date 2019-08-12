//
//  yas_chaining_receiver.h
//

#pragma once

#include "yas_chaining_receiver.h"

namespace yas::chaining {
template <typename T = std::nullptr_t>
struct[[nodiscard]] perform_receiver final : receiver<T> {
    void receive_value(T const &) override;

   private:
    std::function<void(T const &)> _handler;

    perform_receiver(std::function<void(T const &)> const &);
    perform_receiver(std::function<void(T const &)> &&);

   public:
    static std::shared_ptr<perform_receiver> make_shared(std::function<void(T const &)> const &);
    static std::shared_ptr<perform_receiver> make_shared(std::function<void(T const &)> &&);
    static std::shared_ptr<perform_receiver> make_shared(std::function<void(void)> const &);
    static std::shared_ptr<perform_receiver> make_shared(std::function<void(void)> &&);
};

template <typename T>
using perform_receiver_ptr = std::shared_ptr<perform_receiver<T>>;
}  // namespace yas::chaining

#include "yas_chaining_perform_receiver_private.h"
