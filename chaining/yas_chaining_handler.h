//
//  yas_chaining_handler.h
//

#pragma once

#include <chaining/yas_chaining_types.h>

#include <memory>

namespace yas::chaining {
class any_handler;
using any_handler_ptr = std::shared_ptr<any_handler>;

struct handler_impl_base {
    virtual ~handler_impl_base() = default;
};
using handler_impl_base_ptr = std::shared_ptr<handler_impl_base>;

template <typename T>
struct typed_handler_impl : handler_impl_base {
    typed_handler_impl(joint_handler_f<T> &&);

    joint_handler_f<T> const handler;
};
template <typename T>
using typed_handler_impl_ptr = std::shared_ptr<typed_handler_impl<T>>;

struct any_handler {
    template <typename T>
    [[nodiscard]] joint_handler_f<T> const &get() const;

    template <typename T>
    [[nodiscard]] static any_handler_ptr make_shared(joint_handler_f<T> &&);

   private:
    handler_impl_base_ptr _impl_base;

    any_handler(handler_impl_base_ptr const &);
};
}  // namespace yas::chaining

#include <chaining/yas_chaining_handler_private.h>
