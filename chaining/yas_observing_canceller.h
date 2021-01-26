//
//  yas_observing_canceller.h
//

#pragma once

#include <chaining/yas_chaining_invalidatable.h>

#include <cstdint>
#include <functional>
#include <memory>

namespace yas::observing {
class invalidator_pool;
class canceller;
using canceller_ptr = std::shared_ptr<canceller>;
using canceller_wptr = std::weak_ptr<canceller>;
using invalidatable = chaining::invalidatable;
using invalidatable_ptr = std::shared_ptr<invalidatable>;

struct canceller final : invalidatable {
    using remover_f = std::function<void(uint32_t const)>;

    uint32_t const identifier;

    ~canceller();

    void invalidate() override;
    void ignore();
    void add_to(invalidator_pool &pool);

    [[nodiscard]] static canceller_ptr make_shared(uint32_t const identifier, remover_f &&);

   private:
    canceller(uint32_t const identifier, remover_f &&);

    std::weak_ptr<canceller> _weak_canceller;
    std::function<void(uint32_t const)> _handler;
    bool _invalidated = false;
};
}  // namespace yas::observing
