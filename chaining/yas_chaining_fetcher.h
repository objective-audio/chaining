//
//  yas_chaining_fetcher.h
//

#pragma once

#include <chaining/yas_chaining_receiver.h>
#include <chaining/yas_chaining_sender.h>

#include <optional>

namespace yas::chaining {
template <typename T>
class fetcher;

template <typename T>
using fetcher_ptr = std::shared_ptr<fetcher<T>>;

template <typename T>
struct fetcher final : sender<T>, receiver<> {
    std::optional<T> fetched_value() const;

    void push();
    void push(T const &value);

    [[nodiscard]] chain_sync_t<T> chain();

    void receive_value(std::nullptr_t const &) override;

    static fetcher_ptr<T> make_shared(std::function<std::optional<T>(void)>);

   private:
    std::function<std::optional<T>(void)> _fetching_handler;

    explicit fetcher(std::function<std::optional<T>(void)> &&);

    fetcher(fetcher const &) = delete;
    fetcher(fetcher &&) = delete;
    fetcher &operator=(fetcher const &) = delete;
    fetcher &operator=(fetcher &&) = delete;

    void fetch_for(any_joint const &joint) const override;
};
}  // namespace yas::chaining

#include <chaining/yas_chaining_fetcher_private.h>
