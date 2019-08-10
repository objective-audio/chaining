//
//  yas_chaining_value_holder.h
//

#pragma once

#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining::value {
template <typename T>
struct holder final : sender<T>, receiver<T> {
    class impl;

    explicit holder(std::shared_ptr<impl> &&);

    ~holder();

    void set_value(T &&);
    void set_value(T const &);

    [[nodiscard]] T const &raw() const;
    [[nodiscard]] T &raw();

    [[nodiscard]] chain_sync_t<T> chain() const;

    void receive_value(T const &) override;

   private:
    holder(T &&);

    holder(holder const &) = delete;
    holder(holder &&) = delete;
    holder &operator=(holder const &) = delete;
    holder &operator=(holder &&) = delete;

    bool is_equal(sender<T> const &rhs) const override;

   public:
    static std::shared_ptr<holder> make_shared(T);
};
}  // namespace yas::chaining::value

#include "yas_chaining_value_holder_private.h"
