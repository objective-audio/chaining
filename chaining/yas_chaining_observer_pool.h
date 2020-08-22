//
//  yas_chaining_observer_pool.h
//

#pragma once

#include <chaining/yas_chaining_any_observer.h>

#include <unordered_map>

namespace yas::chaining {
class observer_pool;
using observer_pool_ptr = std::shared_ptr<observer_pool>;

struct observer_pool final : invalidatable {
    observer_pool();

    observer_pool(observer_pool &&) = default;
    observer_pool &operator=(observer_pool &&) = default;

    ~observer_pool();

    void add_observer(any_observer_ptr);
    void remove_observer(any_observer_ptr &);

    void invalidate() override;

    observer_pool &operator+=(any_observer_ptr);

    static observer_pool_ptr make_shared();

   private:
    std::unordered_map<std::uintptr_t, any_observer_ptr> _observers;

    observer_pool(observer_pool const &) = delete;
    observer_pool &operator=(observer_pool const &) = delete;
};
}  // namespace yas::chaining
