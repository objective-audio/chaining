//
//  yas_chaining_joint.cpp
//

#include "yas_chaining_joint.h"

using namespace yas::chaining;

any_joint::any_joint(std::shared_ptr<impl> &&ptr) : base(std::move(ptr)) {
}

any_joint::any_joint(std::nullptr_t) : base(nullptr) {
}

void any_joint::fetch() {
    impl_ptr<impl>()->fetch();
}

void any_joint::invalidate() {
    impl_ptr<impl>()->invalidate();
}
