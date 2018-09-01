//
//  yas_chaining_sender.h
//

#pragma once

#include "yas_base.h"

namespace yas::chaining {
template <typename T>
class sendable;

template <typename T>
struct sender : base {
    class impl;

    sender(std::nullptr_t);

    [[nodiscard]] sendable<T> sendable();

   protected:
    sender(std::shared_ptr<impl> &&);

   private:
    chaining::sendable<T> _sendable = nullptr;
};
}  // namespace yas::chaining

#include "yas_chaining_sender_private.h"
