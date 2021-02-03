//
//  yas_observing_canceller.cpp
//

#include "yas_observing_canceller.h"

#include "yas_observing_canceller_pool.h"

using namespace yas;
using namespace yas::observing;

canceller::canceller(uint32_t const identifier, remover_f &&handler)
    : identifier(identifier), _handler(std::move(handler)) {
}

canceller::~canceller() {
    if (!this->_cancelled) {
        this->_handler(this->identifier);
    }
}

void canceller::cancel() {
    if (!this->_cancelled) {
        this->_handler(this->identifier);
        this->_cancelled = true;
    }
}

void canceller::ignore() {
    this->_cancelled = true;
}

bool canceller::has_cancellable() const {
    return !this->_cancelled;
}

void canceller::add_to(canceller_pool &pool) {
    pool.add_canceller(this->_weak_canceller.lock());
}

void canceller::set_to(cancellable_ptr &canceller) {
    canceller = this->_weak_canceller.lock();
}

std::shared_ptr<canceller> canceller::make_shared(uint32_t const identifier, remover_f &&handler) {
    auto shared = canceller_ptr(new canceller{identifier, std::move(handler)});
    shared->_weak_canceller = shared;
    return shared;
}
