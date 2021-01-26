//
//  yas_observing_invalidator_pool.cpp
//

#include "yas_observing_invalidator_pool.h"

using namespace yas;
using namespace yas::observing;

invalidator_pool::~invalidator_pool() {
    this->invalidate();
}

void invalidator_pool::add_invalidator(invalidatable_ptr canceller) {
    assert(this != canceller.get());
    this->_invalidators.emplace_back(std::move(canceller));
}

void invalidator_pool::invalidate() {
    for (auto const &invalidator : this->_invalidators) {
        invalidator->invalidate();
    }
    this->_invalidators.clear();
}

invalidator_pool_ptr invalidator_pool::make_shared() {
    return std::make_shared<invalidator_pool>();
}
