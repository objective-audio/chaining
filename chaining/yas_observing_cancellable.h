//
//  yas_observing_cancellable.h
//

#pragma once

#include <chaining/yas_chaining_invalidatable.h>

namespace yas::observing {
class invalidator_pool;

struct cancellable : chaining::invalidatable {
    void cancel() {
        this->invalidate();
    }

    virtual void add_to(invalidator_pool &) = 0;
    virtual void set_to(std::shared_ptr<cancellable> &) = 0;
};

using cancellable_ptr = std::shared_ptr<cancellable>;
}  // namespace yas::observing
