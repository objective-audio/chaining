//
//  yas_chaining_any_observer.cpp
//

#include "yas_chaining_any_observer.h"

using namespace yas;
using namespace yas::chaining;

any_observer::any_observer(std::nullptr_t) : base(nullptr) {
}

any_observer::any_observer(std::shared_ptr<impl> &&impl) : base(std::move(impl)) {
}

void any_observer::broadcast() {
    impl_ptr<impl>()->broadcast();
}

void any_observer::invalidate() {
    impl_ptr<impl>()->invalidate();
}
