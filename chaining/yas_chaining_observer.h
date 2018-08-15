//
//  yas_chaining_observer.h
//

#pragma once

#include "yas_base.h"

namespace yas::chaining {
template <typename T>
class joint;

struct any_observer : base {
    struct impl : base::impl {
        virtual void broadcast() = 0;
    };

    any_observer(std::nullptr_t) : base(nullptr) {
    }

    void broadcast() {
        impl_ptr<impl>()->broadcast();
    }

   protected:
    any_observer(std::shared_ptr<impl> &&impl) : base(std::move(impl)) {
    }
};

template <typename Begin>
struct observer : any_observer {
    class impl;

    observer(joint<Begin>);
    observer(std::nullptr_t);

    ~observer() final;

    [[nodiscard]] chaining::joint<Begin> &joint();
};
}  // namespace yas::chaining

#include "yas_chaining_observer_private.h"
