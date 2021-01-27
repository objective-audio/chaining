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

    void add_invalidator(invalidatable_ptr);

    void invalidate() override;

    void add_to(invalidator_pool &) override;
    void set_to(invalidatable_ptr &) override;

    [[nodiscard]] static invalidator_pool_ptr make_shared();

   private:
    std::weak_ptr<invalidator_pool> _weak_pool;
    std::vector<invalidatable_ptr> _invalidators;

    invalidator_pool(invalidator_pool const &) = delete;
    invalidator_pool &operator=(invalidator_pool const &) = delete;
};
}  // namespace yas::observing
