//
//  yas_chaining_observer.h
//

#pragma once

#include "yas_chaining_sender_protocol.h"

namespace yas::chaining {
struct observer : base {
    struct impl : base::impl {
        virtual void sync() = 0;
    };

    observer(std::nullptr_t) : base(nullptr) {
    }

    void sync() {
        impl_ptr<impl>()->sync();
    }

   protected:
    observer(std::shared_ptr<impl> &&impl) : base(std::move(impl)) {
    }
};

template <typename Begin>
struct typed_observer : observer {
    class impl;

    typed_observer(joint<Begin>);
    typed_observer(std::nullptr_t);

    ~typed_observer() final;

    [[nodiscard]] chaining::joint<Begin> &joint();
};
}  // namespace yas::chaining

#include "yas_chaining_observer_private.h"
