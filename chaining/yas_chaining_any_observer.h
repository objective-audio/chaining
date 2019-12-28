//
//  yas_chaining_any_observer.h
//

#pragma once

#include "yas_chaining_invalidatable.h"

namespace yas::chaining {
struct any_observer : invalidatable {
    virtual void fetch() = 0;
    virtual void invalidate() = 0;

    virtual ~any_observer();

    uintptr_t identifier() const;

   protected:
    any_observer();

   private:
    any_observer(any_observer const &) = delete;
    any_observer(any_observer &&) = delete;
    any_observer &operator=(any_observer const &) = delete;
    any_observer &operator=(any_observer &&) = delete;
};

using any_observer_ptr = std::shared_ptr<any_observer>;
}  // namespace yas::chaining
