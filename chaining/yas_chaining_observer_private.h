//
//  yas_chaining_observer_private.h
//

#pragma once

#include <vector>
#include "yas_chaining_joint.h"

namespace yas::chaining {
template <typename Begin>
observer<Begin>::observer(std::shared_ptr<chaining::joint<Begin>> joint) : any_observer(), _joint(std::move(joint)) {
}

template <typename Begin>
observer<Begin>::~observer() {
    this->_joint->invalidate();
}

template <typename Begin>
void observer<Begin>::fetch() {
    this->_joint->fetch();
}

template <typename Begin>
void observer<Begin>::invalidate() {
    this->_joint->invalidate();
}

template <typename Begin>
observer_ptr<Begin> observer<Begin>::make_shared(std::shared_ptr<joint<Begin>> joint) {
    auto shared = std::shared_ptr<observer<Begin>>{new observer<Begin>{std::move(joint)}};
    shared->_prepare(shared);
    return shared;
}
}  // namespace yas::chaining
