//
//  yas_chaining_map_holder.h
//

#pragma once

#include <map>
#include "yas_chaining_event.h"
#include "yas_chaining_sender.h"

namespace yas::chaining::map {
template <typename Key, typename Value>
struct fetched_event {
    static event_type const type = event_type::fetched;
    std::map<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct any_event {
    static event_type const type = event_type::any;
    std::map<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct inserted_event {
    static event_type const type = event_type::inserted;
    std::map<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct erased_event {
    static event_type const type = event_type::erased;
    std::map<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct replaced_event {
    static event_type const type = event_type::replaced;
    std::map<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct relayed_event {
    static event_type const type = event_type::relayed;
    Value const &value;
    Key const &key;
    typename Value::SendType const &relayed;
};

template <typename Key, typename Value>
struct immutable_holder : sender<event> {
    class impl;

    immutable_holder(std::nullptr_t);

    using chain_t = chain<event, event, event, true>;

    std::map<Key, Value> const &raw() const;
    bool has_value(Key const &) const;
    Value const &at(Key const &) const;
    std::size_t size() const;

    chain_t chain();

   protected:
    immutable_holder(std::shared_ptr<impl> &&);
};

template <typename Key, typename Value>
struct holder : immutable_holder<Key, Value> {
    holder();
    explicit holder(std::map<Key, Value>);
    holder(std::nullptr_t);

    ~holder() final;

    std::map<Key, Value> &raw();
    Value &at(Key const &);

    void replace_all(std::map<Key, Value>);
    void insert_or_replace(Key, Value);
    void insert(std::map<Key, Value>);
    std::map<Key, Value> erase_if(std::function<bool(Key const &, Value const &)> const &);
    std::map<Key, Value> erase_for_value(Value const &);
    std::map<Key, Value> erase_for_key(Key const &);
    void clear();

   private:
    using immutable_impl = typename immutable_holder<Key, Value>::impl;
};
}  // namespace yas::chaining::map

#include "yas_chaining_map_holder_private.h"
