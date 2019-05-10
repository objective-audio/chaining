//
//  yas_chaining_event.h
//

#pragma once

#include <cpp_utils/yas_base.h>

namespace yas::chaining {
enum class event_type {
    fetched,
    any,
    inserted,
    erased,
    replaced,
    relayed,
};

struct event : base {
    struct impl_base : base::impl {
        virtual event_type type() = 0;
    };

    template <typename Event>
    class impl;

    template <typename Event>
    event(Event &&);
    event(std::nullptr_t);

    [[nodiscard]] event_type type() const;

    template <typename Event>
    [[nodiscard]] Event const &get() const;
};
}  // namespace yas::chaining

#include "yas_chaining_event_private.h"
