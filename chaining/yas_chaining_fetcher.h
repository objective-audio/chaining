//
//  yas_chaining_fetcher.h
//

#pragma once

#include <optional>
#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining {
template <typename T>
struct fetcher final : sender<T>, receiver<> {
    std::optional<T> fetched_value() const;

    void push();
    void push(T const &value);

    [[nodiscard]] chain_sync_t<T> chain();

    void receive_value(std::nullptr_t const &) override;

   private:
    std::function<std::optional<T>(void)> _fetching_handler;

    explicit fetcher(std::function<std::optional<T>(void)> &&);

    fetcher(fetcher const &) = delete;
    fetcher(fetcher &&) = delete;
    fetcher &operator=(fetcher const &) = delete;
    fetcher &operator=(fetcher &&) = delete;

    void fetch_for(any_joint const &joint) override;

   public:
    static std::shared_ptr<fetcher> make_shared(std::function<std::optional<T>(void)>);
};
}  // namespace yas::chaining

#include "yas_chaining_fetcher_private.h"
