//
//  yas_chaining_fetcher.h
//

#pragma once

#include "yas_chaining.h"
#include "yas_chaining_sender.h"

namespace yas::chaining {
template <typename T>
struct fetcher : sender_base<T> {
    class impl;

    fetcher(std::function<opt_t<T>(void)>);
    fetcher(std::nullptr_t);

    void fetch() const;

    [[nodiscard]] chain<T, T, T, true> chain();

    [[nodiscard]] receiver<> &receiver();
};
}  // namespace yas::chaining

#include "yas_chaining_fetcher_private.h"
