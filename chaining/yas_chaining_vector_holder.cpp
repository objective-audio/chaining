//
//  yas_chaining_vector_holder.cpp
//

#include "yas_chaining_vector_holder.h"

namespace yas::chaining::vector {
event::event(std::nullptr_t) : base(nullptr) {
}

event_type event::type() const {
    return this->template impl_ptr<impl_base>()->type();
}
}  // namespace yas::chaining::vector
