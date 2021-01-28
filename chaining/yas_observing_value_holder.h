//
//  yas_observing_value_holder.h
//

#pragma once

#include <chaining/yas_observing_caller.h>

namespace yas::observing::value {
template <typename T>
class holder;

template <typename T>
using holder_ptr = std::shared_ptr<holder<T>>;

template <typename T>
struct holder final {
    void set_value(T &&);
    void set_value(T const &);

    [[nodiscard]] T const &value() const;

    [[nodiscard]] canceller_ptr observe(typename caller<T>::handler_f &&, bool const sync);

    [[nodiscard]] static holder_ptr<T> make_shared(T const &);
    [[nodiscard]] static holder_ptr<T> make_shared(T &&);

   private:
    T _value;
    caller<T> _caller;

    holder(T &&);
};
}  // namespace yas::observing::value

#include <chaining/yas_observing_value_holder_private.h>
