//
//  yas_chaining_any_observer.h
//

#pragma once

#include "yas_chaining_invalidatable.h"

namespace yas::chaining {
class any_observer;
class observer_pool;

using any_observer_ptr = std::shared_ptr<any_observer>;

struct any_observer : invalidatable {
    virtual void fetch() = 0;
    virtual void invalidate() = 0;

    virtual ~any_observer();

    uintptr_t identifier() const;

    void add_to(observer_pool &);

   protected:
    std::weak_ptr<any_observer> _weak_observer;

    any_observer();

    void _prepare(any_observer_ptr const &);

   private:
    any_observer(any_observer const &) = delete;
    any_observer(any_observer &&) = delete;
    any_observer &operator=(any_observer const &) = delete;
    any_observer &operator=(any_observer &&) = delete;
};
}  // namespace yas::chaining
