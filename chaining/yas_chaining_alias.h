//
//  yas_chaining_alias.h
//

#pragma once

#include <cpp_utils/yas_base.h>

namespace yas::chaining {
template <typename T>
struct alias : base {
    class impl;

    explicit alias(T);
    alias(std::nullptr_t);

    [[nodiscard]] auto const &raw() const;
    [[nodiscard]] auto &raw();
    [[nodiscard]] auto chain();
};
}  // namespace yas::chaining

namespace yas {
template <typename T>
[[nodiscard]] chaining::alias<T> make_alias(T);
}

#include "yas_chaining_alias_private.h"
