//
//  yas_observing_invalidatable.h
//

#pragma once

#include <chaining/yas_chaining_invalidatable.h>

namespace yas::observing {
class invalidator_pool;

struct invalidatable : chaining::invalidatable {
    virtual void add_to(invalidator_pool &) = 0;
};

using invalidatable_ptr = std::shared_ptr<invalidatable>;
}  // namespace yas::observing
