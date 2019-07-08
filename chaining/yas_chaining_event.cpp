//
//  yas_chaining_event.cpp
//

#include "yas_chaining_event.h"

namespace yas::chaining {
event_type event::type() const {
    return this->_impl->type();
}
}  // namespace yas::chaining
