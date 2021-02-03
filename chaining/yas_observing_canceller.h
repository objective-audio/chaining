//
//  yas_observing_canceller.h
//

#pragma once

#include <chaining/yas_observing_cancellable.h>

#include <cstdint>
#include <functional>
#include <memory>

namespace yas::observing {
class canceller_pool;
class canceller;
using canceller_ptr = std::shared_ptr<canceller>;
using canceller_wptr = std::weak_ptr<canceller>;

struct canceller final : cancellable {
    using remover_f = std::function<void(uint32_t const)>;

    uint32_t const identifier;

    ~canceller();

    void cancel() override;
    void ignore();
    bool has_cancellable() const override;
    void add_to(canceller_pool &) override;
    void set_to(cancellable_ptr &) override;

    [[nodiscard]] static canceller_ptr make_shared(uint32_t const identifier, remover_f &&);

   private:
    canceller(uint32_t const identifier, remover_f &&);

    std::weak_ptr<canceller> _weak_canceller;
    std::function<void(uint32_t const)> _handler;
    bool _invalidated = false;
};
}  // namespace yas::observing
