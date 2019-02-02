//
//  yas_chaining_map_holder.h
//

#pragma once

#include <map>
#include "yas_chaining_event.h"
#include "yas_chaining_sender.h"

namespace yas::chaining::multimap {
template <typename Key, typename Value>
struct fetched_event {
    static event_type const type = event_type::fetched;
    std::multimap<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct any_event {
    static event_type const type = event_type::any;
    std::multimap<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct inserted_event {
    static event_type const type = event_type::inserted;
    std::multimap<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct erased_event {
    static event_type const type = event_type::erased;
    std::multimap<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct replaced_event {
    static event_type const type = event_type::replaced;
    Key const &key;
    Value const &value;
};

template <typename Key, typename Value>
struct relayed_event {
    static event_type const type = event_type::relayed;
    Key const &key;
    Value const &value;
    typename Value::SendType const &relayed;
};

template <typename Key, typename Value>
struct holder : sender<event> {
    class impl;

    using chain_t = chain<event, event, event, true>;

    holder();
    explicit holder(std::multimap<Key, Value>);
    holder(std::nullptr_t);

    ~holder() final;

    std::multimap<Key, Value> const &raw() const;

    std::size_t size() const;
    void replace(std::multimap<Key, Value>);
    void insert(std::multimap<Key, Value>);
    void insert(Key, Value);
    std::multimap<Key, Value> erase_if(std::function<bool(Key const &, Value const &)> const &);
    std::multimap<Key, Value> erase_for_value(Value const &);
    std::multimap<Key, Value> erase_for_key(Key const &);
    void clear();

    chain_t chain();
};
}  // namespace yas::chaining::multimap

#include "yas_chaining_multimap_holder_private.h"
