//
//  yas_observing_canceller_pool.h
//

#pragma once

#include <chaining/yas_observing_canceller.h>

#include <vector>

namespace yas::observing {
class canceller_pool;
using canceller_pool_ptr = std::shared_ptr<canceller_pool>;

struct canceller_pool : cancellable {
    canceller_pool() = default;

    canceller_pool(canceller_pool &&) = default;
    canceller_pool &operator=(canceller_pool &&) = default;

    ~canceller_pool();

    void add_canceller(cancellable_ptr);

    void invalidate() override;

    bool has_cancellable() const override;

    void add_to(canceller_pool &) override;
    void set_to(cancellable_ptr &) override;

    [[nodiscard]] static canceller_pool_ptr make_shared();

   private:
    std::weak_ptr<canceller_pool> _weak_pool;
    std::vector<cancellable_ptr> _cancellers;

    canceller_pool(canceller_pool const &) = delete;
    canceller_pool &operator=(canceller_pool const &) = delete;
};
}  // namespace yas::observing
