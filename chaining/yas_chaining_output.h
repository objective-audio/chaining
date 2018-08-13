//
//  yas_chaining_output.h
//

#pragma once

#include "yas_base.h"

namespace yas::chaining {
template <typename T>
struct output : base {
    class impl;

    output(weak<receiver<T>>);
    output(std::nullptr_t);

    void output_value(T const &);
};
}  // namespace yas::chaining

#include "yas_chaining_output_private.h"
