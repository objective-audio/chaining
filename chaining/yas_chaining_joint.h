//
//  yas_chaining_joint.h
//

#pragma once

#include <cpp_utils/yas_base.h>
#include <any>

namespace yas::chaining {
template <typename T>
class sender;

struct any_joint : base {
    struct impl : base::impl {
        virtual void fetch() = 0;
        virtual void invalidate() = 0;
        virtual std::any const &handler(std::size_t const idx) = 0;
    };

    any_joint(std::shared_ptr<impl> &&ptr);
    any_joint(std::nullptr_t);

    void fetch();
    void invalidate();

    template <typename T>
    using handler_f = std::function<void(T const &, any_joint &)>;

    template <typename P>
    [[nodiscard]] handler_f<P> const &handler(std::size_t const idx) const;
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
    void push_handler(any_joint::handler_f<P>);
    [[nodiscard]] std::size_t handlers_size() const;
    void add_sub_joint(any_joint);
};
}  // namespace yas::chaining

#include "yas_chaining_joint_private.h"
