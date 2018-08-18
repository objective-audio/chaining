//
//  yas_chaining_observer_pool.h
//

#pragma once

#include "yas_chaining_any_observer.h"

namespace yas::chaining {
struct observer_pool : base {
    class impl;

    observer_pool();
    observer_pool(std::nullptr_t);

    void add_observer(any_observer);
    void remove_observer(any_observer &);

    void invalidate();
};
}  // namespace yas::chaining
