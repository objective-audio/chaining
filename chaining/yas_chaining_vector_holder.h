//
//  yas_chaining_array_holder.h
//

#pragma once

#include <vector>
#include "yas_chaining_sender.h"

namespace yas::chaining {
template <typename Out, typename In, typename Begin, bool Syncable>
class chain;
template <typename T>
class receiver;
}  // namespace yas::chaining

namespace yas::chaining::vector {
enum event_type {
    fetched,
    any,
    inserted,
    erased,
    replaced,
    relayed,
};

template <typename T>
struct fetched_event {
    static vector::event_type const type = vector::event_type::fetched;
    std::vector<T> const &elements;
};

template <typename T>
struct any_event {
    static vector::event_type const type = vector::event_type::any;
    std::vector<T> const &elements;
};

template <typename T>
struct inserted_event {
    static vector::event_type const type = vector::event_type::inserted;
    T const &element;
    std::size_t const index;
};

template <typename T>
struct erased_event {
    static vector::event_type const type = vector::event_type::erased;
    std::size_t const index;
};

template <typename T>
struct replaced_event {
    static vector::event_type const type = vector::event_type::replaced;
    T const &element;
    std::size_t const index;
};

template <typename T>
struct relayed_event {
    static vector::event_type const type = vector::event_type::relayed;
    T const &element;
    std::size_t const index;
    typename T::SendType const &relayed;
};

struct event : base {
    class impl_base;

    template <typename Event>
    class impl;

    template <typename Event>
    event(Event &&);
    event(std::nullptr_t);

    vector::event_type type() const;

    template <typename Event>
    Event const &get() const;
};

template <typename T>
struct immutable_holder : sender<event> {
    class impl;

    immutable_holder(std::nullptr_t);

    using vector_t = std::vector<T>;
    using event_t = vector::event;
    using chain_t = chain<event_t, event_t, event_t, true>;

    vector_t const &raw() const;
    T const &at(std::size_t const) const;
    std::size_t size() const;

    chain_t chain();

   protected:
    immutable_holder(std::shared_ptr<impl> &&);
};

template <typename T>
struct holder : immutable_holder<T> {
    holder();
    explicit holder(std::vector<T>);
    holder(std::nullptr_t);

    ~holder() final;

    void replace(std::vector<T>);
    void replace(T, std::size_t const);
    void push_back(T);
    void insert(T, std::size_t const);
    T erase_at(std::size_t const);
    void clear();

   private:
    using immutable_impl = typename immutable_holder<T>::impl;
};
}  // namespace yas::chaining::vector

#include "yas_chaining_vector_holder_private.h"
