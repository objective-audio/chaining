//
//  yas_observing_invalidator_pool.h
//

#pragma once

#include <chaining/yas_observing_canceller.h>

#include <vector>

namespace yas::observing {
class invalidator_pool;
using invalidator_pool_ptr = std::shared_ptr<invalidator_pool>;

struct invalidator_pool : invalidatable {
    invalidator_pool() = default;

    invalidator_pool(invalidator_pool &&) = default;
    invalidator_pool &operator=(invalidator_pool &&) = default;

    ~invalidator_pool();

    void add_canceller(canceller_ptr);

    void invalidate() override;

    [[nodiscard]] static invalidator_pool_ptr make_shared();

   private:
    std::vector<canceller_ptr> _cancellers;

    invalidator_pool(invalidator_pool const &) = delete;
    invalidator_pool &operator=(invalidator_pool const &) = delete;
};
}  // namespace yas::observing
