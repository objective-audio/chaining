//
//  yas_chaining_fetcher.h
//

#pragma once

#include <optional>
#include "yas_chaining_fetcher_protocol.h"
#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining {
template <typename T>
struct fetcher : sender<T>, receiver<> {
    class impl;

    fetcher(std::function<std::optional<T>(void)>);
    fetcher(std::nullptr_t);

    std::optional<T> fetched_value() const;

    void broadcast() const;
    void broadcast(T const &) const;

    [[nodiscard]] chain_sync_t<T> chain() const;

    [[nodiscard]] chaining::receivable<std::nullptr_t> receivable() override;

    [[nodiscard]] fetchable<T> fetchable();

   private:
    chaining::fetchable<T> _fetchable = nullptr;
    chaining::receivable<std::nullptr_t> _receivable = nullptr;
};
}  // namespace yas::chaining

#include "yas_chaining_fetcher_private.h"
