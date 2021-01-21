//
//  yas_observing_caller.h
//

#pragma once

#include <chaining/yas_observing_canceller.h>

#include <map>
#include <vector>

namespace yas::observing {
template <typename T>
struct caller {
    using handler_f = std::function<void(T const &)>;

    ~caller();

    [[nodiscard]] canceller_ptr add(handler_f &&);
    void call(T const &);

   private:
    struct handler_container {
        bool enabled = true;
        handler_f handler;
    };

    uint32_t _next_idx = 0;
    std::map<uint32_t, handler_container> _handlers;
    std::vector<canceller_wptr> _cancellers;
};
}  // namespace yas::observing

#include <chaining/yas_observing_caller_private.h>
