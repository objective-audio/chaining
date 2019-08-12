//
//  yas_chaining_value_holder.h
//

#pragma once

#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining::value {
template <typename T>
struct holder final : sender<T>, receiver<T> {
    ~holder();

    void set_value(T &&);
    void set_value(T const &);

    [[nodiscard]] T const &raw() const;
    [[nodiscard]] T &raw();

    [[nodiscard]] chain_sync_t<T> chain();

    void receive_value(T const &) override;

   private:
    T _value;
    std::mutex _set_mutex;

    holder(T &&);

    holder(holder const &) = delete;
    holder(holder &&) = delete;
    holder &operator=(holder const &) = delete;
    holder &operator=(holder &&) = delete;

    void fetch_for(any_joint const &joint) const override;

   public:
    static std::shared_ptr<holder> make_shared(T);
};

template <typename T>
using holder_ptr = std::shared_ptr<value::holder<T>>;
}  // namespace yas::chaining::value

#include "yas_chaining_value_holder_private.h"
