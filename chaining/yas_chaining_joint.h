//
//  yas_chaining_joint.h
//

#pragma once

#include <cpp_utils/yas_base.h>
#include <any>

namespace yas::chaining {
template <typename T>
class sender;
template <typename Out, typename In, typename Begin, bool Syncable>
class chain;

struct any_joint : base {
    struct impl : base::impl {
        virtual void fetch() = 0;
        virtual void invalidate() = 0;
        virtual std::any const &handler(std::size_t const idx) = 0;
    };

    any_joint(std::shared_ptr<impl> &&ptr) : base(std::move(ptr)) {
    }

    any_joint(std::nullptr_t) : base(nullptr) {
    }

    void fetch() {
        impl_ptr<impl>()->fetch();
    }

    void invalidate() {
        impl_ptr<impl>()->invalidate();
    }

    template <typename P>
    [[nodiscard]] std::function<void(P const &)> const &handler(std::size_t const idx) const {
        return *std::any_cast<std::function<void(P const &)>>(&impl_ptr<impl>()->handler(idx));
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
    [[nodiscard]] std::size_t handlers_size() const;
    void add_sub_joint(any_joint);
};
}  // namespace yas::chaining

#include "yas_chaining_joint_private.h"
