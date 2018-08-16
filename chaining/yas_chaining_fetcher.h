//
//  yas_chaining_fetcher.h
//

#pragma once

#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"
#include "yas_types.h"

namespace yas::chaining {
template <typename Out, typename In, typename Begin, bool Syncable>
class chain;

template <typename T>
struct fetcher : sender<T> {
    class impl;

    fetcher(std::function<opt_t<T>(void)>);
    fetcher(std::nullptr_t);

    opt_t<T> fetched_value() const;

    void broadcast() const;

    [[nodiscard]] chain<T, T, T, true> chain();

    [[nodiscard]] receiver<> &receiver();
};
}  // namespace yas::chaining

#include "yas_chaining_fetcher_private.h"
