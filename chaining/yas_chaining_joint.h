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

    any_joint(std::shared_ptr<impl> &&ptr);
    any_joint(std::nullptr_t);

    void fetch();
    void invalidate();

    template <typename P>
    [[nodiscard]] std::function<void(P const &)> const &handler(std::size_t const idx) const;
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
