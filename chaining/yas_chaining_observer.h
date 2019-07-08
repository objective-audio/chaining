//
//  yas_chaining_observer.h
//

#pragma once

#include "yas_chaining_any_observer.h"

namespace yas::chaining {
template <typename T>
class joint;

template <typename Begin>
struct[[nodiscard]] observer final : any_observer {
    observer(std::shared_ptr<joint<Begin>>);
    ~observer();

    virtual void fetch() override;
    virtual void invalidate() override;

   private:
    std::shared_ptr<chaining::joint<Begin>> _joint;
};

template <typename Begin>
using observer_ptr = std::shared_ptr<observer<Begin>>;

template <typename Begin>
std::shared_ptr<observer<Begin>> make_observer(joint<Begin>);
}  // namespace yas::chaining

#include "yas_chaining_observer_private.h"
