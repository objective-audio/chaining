//
//  yas_chaining_joint.h
//

#pragma once

#include "yas_base.h"

namespace yas::chaining {
template <typename T>
class sender_base;

struct joint_base : base {
    struct impl : base::impl {
        virtual void sync() = 0;
    };

    joint_base(std::shared_ptr<impl> &&ptr) : base(std::move(ptr)) {
    }

    joint_base(std::nullptr_t) : base(nullptr) {
    }

    void sync() {
        impl_ptr<impl>()->sync();
    }
};

template <typename T>
struct joint : joint_base {
    class impl;

    joint(weak<sender_base<T>>);
    joint(std::nullptr_t);

    ~joint() final;

    void call_first(T const &);

    template <bool Syncable>
    [[nodiscard]] chain<T, T, T, Syncable> begin();

    template <typename P>
    void push_handler(std::function<void(P const &)>);
    std::size_t handlers_size() const;
    template <typename P>
    std::function<void(P const &)> const &handler(std::size_t const) const;
    void add_sub_joint(joint_base);
};
}  // namespace yas::chaining

#include "yas_chaining_joint_private.h"
