//
//  yas_chaining_observer_pool.cpp
//

#include "yas_chaining_observer_pool.h"

using namespace yas;
using namespace yas::chaining;

observer_pool::observer_pool() = default;

observer_pool::~observer_pool() {
    this->invalidate();
}

void observer_pool::add_observer(any_observer_ptr observer) {
    this->_observers.emplace(observer->identifier(), std::move(observer));
}

void observer_pool::remove_observer(any_observer_ptr &observer) {
    if (this->_observers.count(observer->identifier())) {
        observer->invalidate();
        this->_observers.erase(observer->identifier());
    }
}

void observer_pool::invalidate() {
    for (auto &pair : this->_observers) {
        pair.second->invalidate();
    }

    this->_observers.clear();
}

observer_pool &observer_pool::operator+=(any_observer_ptr observer) {
    this->add_observer(std::move(observer));
    return *this;
}

observer_pool_ptr observer_pool::make_shared() {
    return std::make_shared<observer_pool>();
}
