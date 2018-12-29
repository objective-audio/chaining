//
//  yas_chaining_map_holder.h
//

#pragma once

#include <map>
#include "yas_chaining_sender.h"

namespace yas::chaining::multimap {
enum event_type {
    fetched,
    any,
    inserted,
    erased,
    relayed,
};

template <typename Key, typename Value>
struct event : base {
    class impl;

    event(multimap::event_type const, std::multimap<Key, Value> const &);
    event(std::nullptr_t);

    multimap::event_type type() const;
    std::multimap<Key, Value> const &elements() const;

    template <typename Event>
    Event const &get() const;
};

template <typename Key, typename Value>
struct immutable_holder : sender<event<Key, Value>> {
    class impl;

    immutable_holder(std::nullptr_t);

    using event_t = multimap::event<Key, Value>;
    using chain_t = chain<event_t, event_t, event_t, true>;

    std::multimap<Key, Value> const &raw() const;
    std::size_t size() const;

    chain_t chain();

   protected:
    immutable_holder(std::shared_ptr<impl> &&);
};

template <typename Key, typename Value>
struct holder : immutable_holder<Key, Value> {
    holder();
    explicit holder(std::multimap<Key, Value>);
    holder(std::nullptr_t);

    ~holder() final;

    std::multimap<Key, Value> &raw();

    void replace(std::multimap<Key, Value>);
    void insert(std::multimap<Key, Value>);
    void insert(Key, Value);
    std::multimap<Key, Value> erase_if(std::function<bool(Key const &, Value const &)> const &);
    std::multimap<Key, Value> erase_for_value(Value const &);
    std::multimap<Key, Value> erase_for_key(Key const &);
    void clear();

   private:
    using immutable_impl = typename immutable_holder<Key, Value>::impl;
};
}  // namespace yas::chaining::multimap

#include "yas_chaining_multimap_holder_private.h"
