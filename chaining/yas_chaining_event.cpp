//
//  yas_chaining_event.cpp
//

#include "yas_chaining_event.h"

namespace yas::chaining {
event::event(std::nullptr_t) : base(nullptr) {
}

event_type event::type() const {
    return this->template impl_ptr<impl_base>()->type();
}
}  // namespace yas::chaining
