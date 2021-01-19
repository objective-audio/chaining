//
//  yas_observing_canceller_pool.h
//

#pragma once

#include <chaining/yas_observing_canceller.h>

#include <vector>

namespace yas::observing {
class canceller_pool;
using canceller_pool_ptr = std::shared_ptr<canceller_pool>;

struct canceller_pool : invalidatable {
    canceller_pool() = default;

    canceller_pool(canceller_pool &&) = default;
    canceller_pool &operator=(canceller_pool &&) = default;

    ~canceller_pool();

    void add_canceller(canceller_ptr);

    void invalidate() override;

    [[nodiscard]] static canceller_pool_ptr make_shared();

   private:
    std::vector<canceller_ptr> _cancellers;

    canceller_pool(canceller_pool const &) = delete;
    canceller_pool &operator=(canceller_pool const &) = delete;
};
}  // namespace yas::observing
