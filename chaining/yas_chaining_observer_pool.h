//
//  yas_chaining_observer_pool.h
//

#pragma once

#include <unordered_map>
#include "yas_chaining_any_observer.h"

namespace yas::chaining {
struct observer_pool final {
    observer_pool();

    observer_pool(observer_pool &&) = default;
    observer_pool &operator=(observer_pool &&) = default;

    ~observer_pool();

    void add_observer(any_observer_ptr);
    void remove_observer(any_observer_ptr &);

    void invalidate();

    observer_pool &operator+=(any_observer_ptr);

   private:
    std::unordered_map<std::uintptr_t, any_observer_ptr> _observers;

    observer_pool(observer_pool const &) = delete;
    observer_pool &operator=(observer_pool const &) = delete;
};
}  // namespace yas::chaining
