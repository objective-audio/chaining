//
//  yas_chaining_observer_private.h
//

#pragma once

#include <vector>
#include "yas_any.h"
#include "yas_chaining_joint.h"

namespace yas::chaining {
template <typename Begin>
struct typed_observer<Begin>::impl : observer::impl {
    impl(chaining::joint<Begin> &&joint) : _joint(std::move(joint)) {
    }

    void sync() override {
        this->_joint.broadcast();
    }

    chaining::joint<Begin> _joint;
};

template <typename Begin>
typed_observer<Begin>::typed_observer(chaining::joint<Begin> joint)
    : observer(std::make_shared<impl>(std::move(joint))) {
}

template <typename Begin>
typed_observer<Begin>::typed_observer(std::nullptr_t) : observer(nullptr) {
}

template <typename Begin>
typed_observer<Begin>::~typed_observer() = default;

template <typename Begin>
chaining::joint<Begin> &typed_observer<Begin>::joint() {
    return impl_ptr<impl>()->_joint;
}
}  // namespace yas::chaining
