//
//  yas_chaining_invalidatable.h
//

#pragma once

#include <memory>

namespace yas::chaining {
struct invalidatable {
    virtual ~invalidatable() = default;

    virtual void invalidate() = 0;
};

using invalidatable_ptr = std::shared_ptr<invalidatable>;
}  // namespace yas::chaining
