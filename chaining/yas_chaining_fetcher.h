//
//  yas_chaining_fetcher.h
//

#pragma once

#include <optional>
#include "yas_chaining_fetcher_protocol.h"
#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining {
template <typename Out, typename In, typename Begin, bool Syncable>
class chain;

template <typename T>
struct fetcher : sender<T> {
    class impl;

    fetcher(std::function<std::optional<T>(void)>);
    fetcher(std::nullptr_t);

    std::optional<T> fetched_value() const;

    void broadcast() const;
    void broadcast(T const &) const;

    [[nodiscard]] chain_sync_t<T> chain();

    [[nodiscard]] receiver<> &receiver();

    [[nodiscard]] fetchable<T> fetchable();

   private:
    chaining::fetchable<T> _fetchable = nullptr;
};
}  // namespace yas::chaining

#include "yas_chaining_fetcher_private.h"
