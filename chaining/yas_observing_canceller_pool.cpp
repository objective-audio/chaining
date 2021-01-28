//
//  yas_observing_canceller_pool.cpp
//

#include "yas_observing_canceller_pool.h"

using namespace yas;
using namespace yas::observing;

canceller_pool::~canceller_pool() {
    this->invalidate();
}

void canceller_pool::add_canceller(cancellable_ptr canceller) {
    assert(this != canceller.get());
    this->_cancellers.emplace_back(std::move(canceller));
}

void canceller_pool::invalidate() {
    for (auto const &canceller : this->_cancellers) {
        canceller->cancel();
    }
    this->_cancellers.clear();
}

void canceller_pool::add_to(canceller_pool &pool) {
    pool.add_canceller(this->_weak_pool.lock());
}

void canceller_pool::set_to(cancellable_ptr &invalidator) {
    invalidator = this->_weak_pool.lock();
}

canceller_pool_ptr canceller_pool::make_shared() {
    auto shared = std::make_shared<canceller_pool>();
    shared->_weak_pool = shared;
    return shared;
}
