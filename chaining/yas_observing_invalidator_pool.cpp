//
//  yas_observing_invalidator_pool.cpp
//

#include "yas_observing_invalidator_pool.h"

using namespace yas;
using namespace yas::observing;

invalidator_pool::~invalidator_pool() {
    this->invalidate();
}

void invalidator_pool::add_canceller(canceller_ptr canceller) {
    this->_cancellers.emplace_back(std::move(canceller));
}

void invalidator_pool::invalidate() {
    for (auto const &canceller : this->_cancellers) {
        canceller->invalidate();
    }
    this->_cancellers.clear();
}

invalidator_pool_ptr invalidator_pool::make_shared() {
    return std::make_shared<invalidator_pool>();
}
