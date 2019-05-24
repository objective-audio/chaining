//
//  yas_chaining_array_holder.h
//

#pragma once

#include <vector>
#include "yas_chaining_event.h"
#include "yas_chaining_sender.h"

namespace yas::chaining {
template <typename T>
class receiver;
}  // namespace yas::chaining

namespace yas::chaining::vector {
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
struct holder : sender<event> {
    class impl;

    using vector_t = std::vector<T>;
    using chain_t = chain<event, event, true>;

    holder();
    explicit holder(vector_t);
    holder(std::nullptr_t);

    ~holder() final;

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

    [[nodiscard]] receiver<event> &receiver();
};
}  // namespace yas::chaining::vector

#include "yas_chaining_vector_holder_private.h"
