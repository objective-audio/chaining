//
//  yas_chaining_joint.cpp
//

#include "yas_chaining_joint.h"

using namespace yas::chaining;

any_joint::any_joint() = default;

any_joint::~any_joint() = default;

uintptr_t any_joint::identifier() const {
    return reinterpret_cast<uintptr_t>(this);
}
