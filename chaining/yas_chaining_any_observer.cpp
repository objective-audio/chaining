//
//  yas_chaining_any_observer.cpp
//

#include "yas_chaining_any_observer.h"
#include "yas_chaining_observer_pool.h"

using namespace yas;
using namespace yas::chaining;

any_observer::any_observer() = default;

any_observer::~any_observer() = default;

uintptr_t any_observer::identifier() const {
    return reinterpret_cast<uintptr_t>(this);
}

void any_observer::add_to(observer_pool &pool) {
    pool.add_observer(this->_weak_observer.lock());
}

void any_observer::_prepare(any_observer_ptr const &shared) {
    this->_weak_observer = shared;
}
