//
//  yas_chaining_event.h
//

#pragma once

#include <memory>

namespace yas::chaining {
enum class event_type {
    fetched,
    any,
    inserted,
    erased,
    replaced,
    relayed,
};

struct event {
    struct impl_base {
        virtual event_type type() = 0;
    };

    template <typename Event>
    class impl;

    template <typename Event>
    event(Event &&);

    [[nodiscard]] event_type type() const;

    template <typename Event>
    [[nodiscard]] Event const &get() const;

   private:
    std::shared_ptr<impl_base> _impl;
};
}  // namespace yas::chaining

#include "yas_chaining_event_private.h"
