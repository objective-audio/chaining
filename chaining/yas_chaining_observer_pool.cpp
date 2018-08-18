//
//  yas_chaining_observer_pool.cpp
//

#include "yas_chaining_observer_pool.h"
#include <unordered_map>

using namespace yas;
using namespace yas::chaining;

struct observer_pool::impl : base::impl {
    ~impl() {
        this->invalidate();
    }

    void add_observer(any_observer &&observer) {
        this->_observers.emplace(observer.identifier(), std::move(observer));
    }

    void remove_observer(any_observer const &observer) {
        if (this->_observers.count(observer.identifier())) {
            this->_observers.erase(observer.identifier());
        }
    }

    void invalidate() {
        for (auto &pair : this->_observers) {
            pair.second.invalidate();
        }

        this->_observers.clear();
    }

   private:
    std::unordered_map<std::uintptr_t, any_observer> _observers;
};

observer_pool::observer_pool() : base(std::make_shared<impl>()) {
}

observer_pool::observer_pool(std::nullptr_t) : base(nullptr) {
}

void observer_pool::add_observer(any_observer observer) {
    impl_ptr<impl>()->add_observer(std::move(observer));
}

void observer_pool::remove_observer(any_observer const &observer) {
    impl_ptr<impl>()->remove_observer(observer);
}

void observer_pool::invalidate() {
    impl_ptr<impl>()->invalidate();
}
