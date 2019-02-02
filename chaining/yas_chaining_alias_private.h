//
//  yas_chaining_alias_private.h
//

#pragma once

namespace yas::chaining {
template <typename T>
struct alias<T>::impl : base::impl {
    T _sender;

    impl(T &&sender) : _sender(sender) {
    }
};

template <typename T>
alias<T>::alias(T sender) : base(std::make_shared<impl>(std::move(sender))) {
}

template <typename T>
alias<T>::alias(std::nullptr_t) : base(nullptr) {
}

template <typename T>
auto const &alias<T>::raw() const {
    return impl_ptr<impl>()->_sender.raw();
}

template <typename T>
auto alias<T>::chain() {
    return impl_ptr<impl>()->_sender.chain();
}
}  // namespace yas::chaining

namespace yas {
template <typename T>
yas::chaining::alias<T> make_alias(T sender) {
    return chaining::alias{std::move(sender)};
}
}  // namespace yas
