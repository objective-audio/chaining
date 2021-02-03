//
//  yas_observing_cancellable.h
//

#pragma once

#include <memory>

namespace yas::observing {
class canceller_pool;

struct cancellable {
    virtual ~cancellable() = default;

    virtual void cancel() = 0;
    virtual bool has_cancellable() const = 0;
    virtual void add_to(canceller_pool &) = 0;
    virtual void set_to(std::shared_ptr<cancellable> &) = 0;
};

using cancellable_ptr = std::shared_ptr<cancellable>;
}  // namespace yas::observing
