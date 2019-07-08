//
//  yas_chaining_any_observer.cpp
//

#include "yas_chaining_any_observer.h"

using namespace yas;
using namespace yas::chaining;

any_observer::any_observer() = default;

any_observer::~any_observer() = default;

uintptr_t any_observer::identifier() const {
    return reinterpret_cast<uintptr_t>(this);
}
