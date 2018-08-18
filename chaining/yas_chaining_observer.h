//
//  yas_chaining_observer.h
//

#pragma once

#include "yas_chaining_any_observer.h"

namespace yas::chaining {
template <typename T>
class joint;

template <typename Begin>
struct observer : any_observer {
    class impl;

    observer(joint<Begin>);
    observer(std::nullptr_t);

    ~observer() final;

    [[nodiscard]] chaining::joint<Begin> &joint();
};
}  // namespace yas::chaining

#include "yas_chaining_observer_private.h"
