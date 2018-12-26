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
struct fetched_event {
    static multimap::event_type const type = multimap::event_type::fetched;
    std::multimap<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct any_event {
    static multimap::event_type const type = multimap::event_type::any;
    std::multimap<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct inserted_event {
    static multimap::event_type const type = multimap::event_type::inserted;
    std::multimap<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct erased_event {
    static multimap::event_type const type = multimap::event_type::erased;
    std::multimap<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct relayed_event {
    static multimap::event_type const type = multimap::event_type::relayed;
    Value const &value;
    Key const &key;
};

template <typename Key, typename Value>
struct event : base {
    class impl_base;

    template <typename Event>
    class impl;

    event(fetched_event<Key, Value> &&);
    event(any_event<Key, Value> &&);
    event(inserted_event<Key, Value> &&);
    event(erased_event<Key, Value> &&);
    event(relayed_event<Key, Value> &&);
    event(std::nullptr_t);

    multimap::event_type type() const;

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
