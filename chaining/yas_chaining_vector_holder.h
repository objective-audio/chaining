//
//  yas_chaining_array_holder.h
//

#pragma once

#include <cpp_utils/yas_weakable.h>
#include <vector>
#include "yas_chaining_any_observer.h"
#include "yas_chaining_event.h"
#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining::vector {
struct event : chaining::event {
    template <typename Event>
    event(Event &&event) : chaining::event(std::move(event)) {
    }
};

template <typename T>
struct fetched_event {
    static event_type const type = event_type::fetched;
    std::vector<T> const &elements;
};

template <typename T>
struct any_event {
    static event_type const type = event_type::any;
    std::vector<T> const &elements;
};

template <typename T>
struct inserted_event {
    static event_type const type = event_type::inserted;
    T const &element;
    std::size_t const index;
};

template <typename T>
struct erased_event {
    static event_type const type = event_type::erased;
    std::size_t const index;
};

template <typename T>
struct replaced_event {
    static event_type const type = event_type::replaced;
    T const &element;
    std::size_t const index;
};

template <typename T>
struct relayed_event {
    static event_type const type = event_type::relayed;
    T const &element;
    std::size_t const index;
    typename T::SendType const &relayed;
};

template <typename T>
struct holder final : sender<event>, receiver<event>, weakable<holder<T>> {
    class impl;

    using vector_t = std::vector<T>;
    using chain_t = chain<event, event, true>;

    explicit holder(std::shared_ptr<impl> &&);

    ~holder();

    [[nodiscard]] vector_t const &raw() const;
    [[nodiscard]] vector_t &raw();
    [[nodiscard]] T const &at(std::size_t const) const;
    [[nodiscard]] std::size_t size() const;

    void replace(vector_t);
    void replace(T, std::size_t const);
    void push_back(T);
    void insert(T, std::size_t const);
    T erase_at(std::size_t const);
    void clear();

    [[nodiscard]] chain_t chain() const;

    void receive_value(event const &) override;

    std::shared_ptr<weakable_impl> weakable_impl_ptr() const override;

    struct observer_wrapper {
        any_observer_ptr observer = nullptr;
    };

    using wrapper_ptr = std::shared_ptr<observer_wrapper>;
    using wrapper_wptr = std::weak_ptr<observer_wrapper>;
    using chaining_f = std::function<void(T &, wrapper_ptr &)>;

   private:
    explicit holder(vector_t);

    bool is_equal(sender<event> const &rhs) const override;

    void _prepare(std::vector<T> &&);

   public:
    static std::shared_ptr<holder> make_shared();
    static std::shared_ptr<holder> make_shared(vector_t);
};
}  // namespace yas::chaining::vector

#include "yas_chaining_vector_holder_private.h"
