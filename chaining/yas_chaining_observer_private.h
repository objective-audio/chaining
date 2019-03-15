//
//  yas_chaining_observer_private.h
//

#pragma once

#include <vector>
#include "yas_chaining_joint.h"

namespace yas::chaining {
template <typename Begin>
struct observer<Begin>::impl : any_observer::impl {
    impl(chaining::joint<Begin> &&joint) : _joint(std::move(joint)) {
    }

    void fetch() override {
        this->_joint.fetch();
    }

    void invalidate() override {
        this->_joint.invalidate();
    }

    chaining::joint<Begin> _joint;
};

template <typename Begin>
observer<Begin>::observer(chaining::joint<Begin> joint) : any_observer(std::make_shared<impl>(std::move(joint))) {
}

template <typename Begin>
observer<Begin>::observer(std::nullptr_t) : any_observer(nullptr) {
}

template <typename Begin>
observer<Begin>::~observer() = default;

template <typename Begin>
chaining::joint<Begin> &observer<Begin>::joint() {
    return impl_ptr<impl>()->_joint;
}
}  // namespace yas::chaining
