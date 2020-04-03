//
//  yas_chaining_observer.h
//

#pragma once

#include "yas_chaining_any_observer.h"

namespace yas::chaining {
template <typename T>
class joint;

template <typename Begin>
class observer;

template <typename Begin>
using observer_ptr = std::shared_ptr<observer<Begin>>;

template <typename Begin>
struct [[nodiscard]] observer final : any_observer {
    ~observer();

    virtual void fetch() override;
    virtual void invalidate() override;

    static observer_ptr<Begin> make_shared(std::shared_ptr<joint<Begin>>);

   private:
    std::shared_ptr<chaining::joint<Begin>> _joint;

    observer(std::shared_ptr<joint<Begin>>);
};
}  // namespace yas::chaining

#include "yas_chaining_observer_private.h"
