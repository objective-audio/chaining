//
//  yas_chaining_any_observer.h
//

#pragma once

#include <cpp_utils/yas_base.h>

namespace yas::chaining {
struct any_observer : base {
    struct impl : base::impl {
        virtual void broadcast() = 0;
        virtual void invalidate() = 0;
    };

    any_observer(std::nullptr_t);

    void broadcast();
    void invalidate();

   protected:
    any_observer(std::shared_ptr<impl> &&impl);
};
}  // namespace yas::chaining
