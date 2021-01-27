//
//  yas_observing_invalidatable.h
//

#pragma once

#include <chaining/yas_chaining_invalidatable.h>

namespace yas::observing {
struct invalidatable : chaining::invalidatable {};

using invalidatable_ptr = std::shared_ptr<invalidatable>;
}  // namespace yas::observing
