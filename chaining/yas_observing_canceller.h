//
//  yas_observing_canceller.h
//

#pragma once

#include <cstdint>
#include <functional>
#include <memory>

namespace yas::observing {
class canceller;
using canceller_ptr = std::shared_ptr<canceller>;
using canceller_wptr = std::weak_ptr<canceller>;

struct canceller final {
    using remover_f = std::function<void(uint32_t const)>;

    uint32_t const identifier;

    ~canceller();

    void invalidate();
    void ignore();

    [[nodiscard]] static canceller_ptr make_shared(uint32_t const identifier, remover_f &&);

   private:
    canceller(uint32_t const identifier, remover_f &&);

    std::function<void(uint32_t const)> _handler;
    bool _invalidated = false;
};
}  // namespace yas::observing
