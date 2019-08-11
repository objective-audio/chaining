//
//  yas_chaining_sender_private.h
//

#pragma once

#include <cpp_utils/yas_stl_utils.h>
#include <vector>

namespace yas::chaining {
template <typename T>
uintptr_t sender<T>::identifier() const {
    auto shared = this->shared_from_this();
    return reinterpret_cast<uintptr_t>(shared.get());
}

template <typename T>
void sender<T>::fetch_for(any_joint const &joint) {
}
}  // namespace yas::chaining
