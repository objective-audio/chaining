//
//  yas_chaining_handler_private.h
//

#pragma once

namespace yas::chaining {
template <typename T>
typed_handler_impl<T>::typed_handler_impl(joint_handler_f<T> &&handler) : handler(std::move(handler)) {
}

template <typename T>
joint_handler_f<T> const &any_handler::get() const {
    if (typed_handler_impl_ptr<T> const casted = std::dynamic_pointer_cast<typed_handler_impl<T>>(this->_impl_base)) {
        return casted->handler;
    } else {
        static joint_handler_f<T> const _null_handler = nullptr;
        return _null_handler;
    }
}

template <typename T>
any_handler_ptr any_handler::make_shared(joint_handler_f<T> &&handler) {
    return any_handler_ptr{new any_handler{std::make_shared<typed_handler_impl<T>>(std::move(handler))}};
}
}  // namespace yas::chaining
