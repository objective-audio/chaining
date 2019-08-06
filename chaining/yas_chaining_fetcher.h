//
//  yas_chaining_fetcher.h
//

#pragma once

#include <cpp_utils/yas_weakable.h>
#include <optional>
#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining {
template <typename T>
struct fetcher final : sender<T>, receiver<>, weakable<fetcher<T>> {
    class impl;

    explicit fetcher(std::shared_ptr<impl> &&);

    std::optional<T> fetched_value() const;

    void broadcast() const;
    void broadcast(T const &) const;

    [[nodiscard]] chain_sync_t<T> chain() const;

    void receive_value(std::nullptr_t const &) override;
    [[nodiscard]] chaining::receivable_ptr<std::nullptr_t> receivable() override;

    std::shared_ptr<weakable_impl> weakable_impl_ptr() const override;

   private:
    explicit fetcher(std::function<std::optional<T>(void)> &&);

   public:
    static std::shared_ptr<fetcher> make_shared(std::function<std::optional<T>(void)>);
};
}  // namespace yas::chaining

#include "yas_chaining_fetcher_private.h"
