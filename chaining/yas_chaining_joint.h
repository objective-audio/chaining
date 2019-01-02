//
//  yas_chaining_joint.h
//

#pragma once

#include <cpp_utils/yas_base.h>

namespace yas::chaining {
template <typename T>
class sender;
template <typename Out, typename In, typename Begin, bool Syncable>
class chain;

struct any_joint : base {
    struct impl : base::impl {
        virtual void broadcast() = 0;
        virtual void invalidate() = 0;
    };

    any_joint(std::shared_ptr<impl> &&ptr) : base(std::move(ptr)) {
    }

    any_joint(std::nullptr_t) : base(nullptr) {
    }

    void broadcast() {
        impl_ptr<impl>()->broadcast();
    }

    void invalidate() {
        impl_ptr<impl>()->invalidate();
    }
};

template <typename T>
struct[[nodiscard]] joint : any_joint {
    class impl;

    joint(weak<sender<T>>);
    joint(std::nullptr_t);

    ~joint() final;

    void call_first(T const &);
    void invalidate();

    template <typename P>
    void push_handler(std::function<void(P const &)>);
    std::size_t handlers_size() const;
    template <typename P>
    std::function<void(P const &)> const &handler(std::size_t const) const;
    void add_sub_joint(any_joint);
};
}  // namespace yas::chaining

#include "yas_chaining_joint_private.h"
