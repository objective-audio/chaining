//
//  yas_observing_invalidator_pool.cpp
//

#include "yas_observing_invalidator_pool.h"

using namespace yas;
using namespace yas::observing;

invalidator_pool::~invalidator_pool() {
    this->invalidate();
}

void invalidator_pool::add_invalidator(cancellable_ptr canceller) {
    assert(this != canceller.get());
    this->_invalidators.emplace_back(std::move(canceller));
}

void invalidator_pool::invalidate() {
    for (auto const &invalidator : this->_invalidators) {
        invalidator->invalidate();
    }
    this->_invalidators.clear();
}

void invalidator_pool::add_to(invalidator_pool &pool) {
    pool.add_invalidator(this->_weak_pool.lock());
}

void invalidator_pool::set_to(cancellable_ptr &invalidator) {
    invalidator = this->_weak_pool.lock();
}

invalidator_pool_ptr invalidator_pool::make_shared() {
    auto shared = std::make_shared<invalidator_pool>();
    shared->_weak_pool = shared;
    return shared;
}
