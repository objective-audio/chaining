//
//  yas_observing_canceller_pool.cpp
//

#include "yas_observing_canceller_pool.h"

using namespace yas;
using namespace yas::observing;

canceller_pool::~canceller_pool() {
    this->invalidate();
}

void canceller_pool::add_canceller(canceller_ptr canceller) {
    this->_cancellers.emplace_back(std::move(canceller));
}

void canceller_pool::invalidate() {
    for (auto const &canceller : this->_cancellers) {
        canceller->invalidate();
    }
    this->_cancellers.clear();
}

canceller_pool_ptr canceller_pool::make_shared() {
    return std::make_shared<canceller_pool>();
}
